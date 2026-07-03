#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "config.h"
#include "dashboard.html.h"

// UART Link to Brain A
#define LinkSerial Serial1

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Global state from Brain A
JsonDocument lastTelemetry;
bool controlNodeOffline = true;
unsigned long lastValidLinkMessage = 0;
uint32_t sequenceNum = 0;
uint32_t lastReceivedSeq = 0;

// CRC8 calculation
uint8_t crc8(const uint8_t *data, size_t len) {
  uint8_t crc = 0x00;
  while (len--) {
    uint8_t extract = *data++;
    for (uint8_t tempI = 8; tempI; tempI--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) crc ^= 0x8C;
      extract >>= 1;
    }
  }
  return crc;
}

void sendLinkMessage(JsonDocument &payload) {
  JsonDocument envelope;
  envelope["s"] = ++sequenceNum;
  String body;
  serializeJson(payload, body);
  envelope["c"] = crc8((uint8_t*)body.c_str(), body.length());
  envelope["m"] = payload;
  serializeJson(envelope, LinkSerial);
  LinkSerial.println();
}

void handleLink() {
  static String inputBuffer = "";
  // 6. Non-blocking line-buffered reading (512 bytes)
  while (LinkSerial.available()) {
    char c = LinkSerial.read();
    if (c == '\n') {
      JsonDocument envelope;
      DeserializationError err = deserializeJson(envelope, inputBuffer);
      if (!err) {
        // 8. Envelope fields verification (s, c, m)
        uint32_t seq = envelope["s"];
        uint8_t receivedCrc = envelope["c"];
        JsonObject m = envelope["m"];

        String body;
        serializeJson(m, body);
        uint8_t expectedCrc = crc8((uint8_t*)body.c_str(), body.length());

        if (receivedCrc == expectedCrc) {
          // 8. Sequence check (Duplicate/Gap detection)
          if (seq != 0 && seq == lastReceivedSeq) {
            inputBuffer = "";
            continue;
          }
          if (lastReceivedSeq != 0 && seq != lastReceivedSeq + 1) {
            Serial.printf("Link gap detected: %u -> %u\n", lastReceivedSeq, seq);
          }
          lastReceivedSeq = seq;

          lastValidLinkMessage = millis();
          controlNodeOffline = false;

          String type = m["type"];
          if (type == "TELE") {
            lastTelemetry = m;
          }
        } else {
          // 9. Log CRC failure
          Serial.printf("Link CRC error! Recv %02X, Exp %02X\n", receivedCrc, expectedCrc);
        }
      }
      inputBuffer = "";
    } else {
      inputBuffer += c;
      if (inputBuffer.length() > 512) inputBuffer = ""; // Safety flush (6)
    }
  }

  if (millis() - lastValidLinkMessage > LINK_TIMEOUT_MS) {
    controlNodeOffline = true;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  JsonDocument doc;
  deserializeJson(doc, payload, length);
  if (doc["command"].is<String>()) {
    String cmd = doc["command"];
    String key = doc["key"] | "";
    if (key != MQTT_CMD_KEY) return; // Basic validation

    if (cmd == "feed" && !controlNodeOffline) {
      JsonDocument pumpCmd;
      pumpCmd["type"] = "PUMP";
      pumpCmd["pump"] = "nutrient";
      pumpCmd["dur"] = 5000;
      sendLinkMessage(pumpCmd);
    }
  }
}

void setup() {
  Serial.begin(115200);
  LinkSerial.begin(115200, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  pinMode(STATUS_LED, OUTPUT);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });


  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String output;
    if (controlNodeOffline) {
        JsonDocument offlineDoc;
        offlineDoc["lost"] = true;
        offlineDoc["ph"] = 0; offlineDoc["od"] = 0; offlineDoc["temp"] = 0;
        serializeJson(offlineDoc, output);
    } else {
        serializeJson(lastTelemetry, output);
    }
    request->send(200, "application/json", output);
  });

  server.on("/pump", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
    if(!controlNodeOffline && request->hasParam("type", true)) {
      JsonDocument pumpCmd;
      pumpCmd["type"] = "PUMP";
      pumpCmd["pump"] = request->getParam("type", true)->value();
      pumpCmd["dur"] = 5000;
      sendLinkMessage(pumpCmd);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return;
    JsonDocument doc;
    deserializeJson(doc, (const char*)data, len);
    if(!controlNodeOffline) {
      JsonDocument setCmd;
      setCmd["type"] = "SET_T";
      if(doc["phTarget"].is<float>()) setCmd["ph"] = doc["phTarget"];
      if(doc["tempTarget"].is<float>()) setCmd["temp"] = doc["tempTarget"];
      sendLinkMessage(setCmd);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();

  ArduinoOTA.setHostname("bio-connectivity");
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.begin();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);
}

void loop() {
  ArduinoOTA.handle();
  handleLink();

  if (WiFi.status() == WL_CONNECTED) {
    // Update system time via NTP
    static bool timeSynced = false;
    if (!timeSynced) {
        configTime(3600, 3600, "pool.ntp.org");
        timeSynced = true;
    }

    if (!mqttClient.connected()) {
      static unsigned long lastMqttRetry = 0;
      if (millis() - lastMqttRetry > 5000) {
        // 11. Refuse anonymous MQTT connection if credentials are set
        if (strlen(MQTT_USER) > 0) {
            if (mqttClient.connect("BioConnectivity", MQTT_USER, MQTT_PASS)) {
                mqttClient.subscribe(MQTT_TOPIC_PREFIX "/command");
            }
        } else {
            // Broker might not require auth, but we still use CMD_KEY for commands
            if (mqttClient.connect("BioConnectivity")) {
                mqttClient.subscribe(MQTT_TOPIC_PREFIX "/command");
            }
        }
        lastMqttRetry = millis();
      }
    }
    mqttClient.loop();
  }

  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
    JsonDocument hb;
    hb["type"] = "HB";
    time_t now;
    time(&now);
    hb["epoch"] = (now > 1700000000) ? now : 0;
    sendLinkMessage(hb);
    lastHeartbeat = millis();
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));

    if (!controlNodeOffline && mqttClient.connected()) {
        String output;
        serializeJson(lastTelemetry, output);
        mqttClient.publish(MQTT_TOPIC_PREFIX "/telemetry", output.c_str());
    }
  }
}
