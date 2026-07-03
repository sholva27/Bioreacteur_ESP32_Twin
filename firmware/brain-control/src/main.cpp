#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Adafruit_ADS1X15.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <FS.h>
#include <LittleFS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

// Hardware interfaces
Adafruit_ADS1115 ads;
RTC_DS3231 rtc;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

// UART Link to Brain B
#define LinkSerial Serial1

// Global state
float phTarget = PH_TARGET_DEFAULT;
float tempTarget = TEMP_TARGET_DEFAULT;
int stirrerSpeed = STIRRER_SPEED_DEFAULT;
long feedingInterval = FEEDING_INTERVAL_MS_DEFAULT;

float phOffset = 0.0;
float phSlope = 1.0;
float odZeroVoltage = 0.0;

float currentPH = 0.0;
float currentOD = 0.0;
float currentTemp = 0.0;
float currentPH_V = 0.0;
float currentOD_V = 0.0;
float currentUV_V = 0.0;
float currentPressure_V = 0.0;
float growthRate = 0.0;

float lastOD = 0.0;
unsigned long lastODTime = 0;

bool sensorError = false;
bool linkLost = true;
unsigned long lastSensorRead = 0;
unsigned long lastFeedingTime = 0;
unsigned long lastLogTime = 0;
unsigned long lastTelemetrySend = 0;
unsigned long lastHeartbeatSend = 0;
unsigned long lastValidLinkMessage = 0;
unsigned long lastPHAction = 0;
uint32_t sequenceNum = 0;
uint32_t lastReceivedSeq = 0;

// Pump states
struct PumpState {
  int pin;
  bool active;
  unsigned long startTime;
  unsigned long duration;
};

PumpState acidPump = {PUMP_ACID_PIN, false, 0, 0};
PumpState basePump = {PUMP_BASE_PIN, false, 0, 0};
PumpState nutrientPump = {PUMP_NUTRIENT_PIN, false, 0, 0};

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

void writeActuator(int pin, bool active) {
#if RELAY_ACTIVE_LOW
  digitalWrite(pin, active ? LOW : HIGH);
#else
  digitalWrite(pin, active ? HIGH : LOW);
#endif
}

void emergencyStop() {
  writeActuator(PUMP_ACID_PIN, false);
  writeActuator(PUMP_BASE_PIN, false);
  writeActuator(PUMP_NUTRIENT_PIN, false);
  writeActuator(HEATER_PIN, false);
  analogWrite(STIRRER_PIN, 0);
  acidPump.active = false;
  basePump.active = false;
  nutrientPump.active = false;
}

void startPump(PumpState &p, unsigned long duration) {
  if (duration > PUMP_MAX_MS) duration = PUMP_MAX_MS;
  p.duration = duration;
  p.startTime = millis();
  p.active = true;
}

void updatePumps() {
  unsigned long now = millis();
  PumpState* pumps[] = {&acidPump, &basePump, &nutrientPump};
  for (int i = 0; i < 3; i++) {
    if (pumps[i]->active) {
      if (now - pumps[i]->startTime >= pumps[i]->duration) {
        writeActuator(pumps[i]->pin, false);
        pumps[i]->active = false;
      } else {
        writeActuator(pumps[i]->pin, true);
      }
    }
  }
}

bool checkI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

void updateSensors() {
  unsigned long currentMillis = millis();
  sensorError = false;

  if (!checkI2C(0x48)) {
    Serial.println("ADS1115 I2C disconnect!");
    sensorError = true;
  }

  float temp = sensors.getTempCByIndex(0);
  if (temp != DEVICE_DISCONNECTED_C && temp > 0) {
    currentTemp = temp;
  } else {
    sensorError = true;
  }
  sensors.requestTemperatures();

  if (!sensorError) {
    // Read all 4 channels of ADS1115
    // Use GAIN_ONE for ±4.096V range
    currentUV_V = ads.computeVolts(ads.readADC_SingleEnded(ADS_UV_CH));
    currentPressure_V = ads.computeVolts(ads.readADC_SingleEnded(ADS_PRESSURE_CH));
    currentPH_V = ads.computeVolts(ads.readADC_SingleEnded(ADS_PH_CH));

    digitalWrite(OD_LIGHT_PIN, HIGH);
    delayMicroseconds(500);
    currentOD_V = ads.computeVolts(ads.readADC_SingleEnded(ADS_OD_CH));
    digitalWrite(OD_LIGHT_PIN, LOW);

    // 18. Nernst Slope (P4).
    // Ideal value is 59.16 mV/pH at 25°C.
    // Note: Slope must be calibrated at the target service temperature (e.g. 37°C).
    // The control loop uses 37°C as target; manual calibration at this T is mandatory.
    float effectiveSlope_V = (59.16f / 1000.0f) * phSlope;

    // 2. pH Safeguard
    if (abs(effectiveSlope_V) > 0.0001f) {
        currentPH = 7.0f + (PH_VMID - currentPH_V) / effectiveSlope_V + phOffset;
    } else {
        currentPH = NAN;
    }

    if (!isfinite(currentPH)) {
        sensorError = true;
    }

    if (currentOD_V > 0.001f && odZeroVoltage > 0.001f) {
      currentOD = log10(odZeroVoltage / currentOD_V) * OD_CALIBRATION_FACTOR;
    } else {
      currentOD = (currentOD_V <= 0.001) ? 4.0 : 0.0;
    }
  }

  if (sensorError) emergencyStop();

  // Growth rate
  if (lastODTime > 0 && currentOD > 0.05 && lastOD > 0.05) {
     float dt = (currentMillis - lastODTime) / 3600000.0;
     if (dt > 0.1) {
        float mu = (log(currentOD) - log(lastOD)) / dt;
        growthRate = (growthRate * 0.8) + (mu * 0.2);
        lastOD = currentOD;
        lastODTime = currentMillis;
     }
  } else if (currentOD > 0.05 && lastODTime == 0) {
     lastOD = currentOD;
     lastODTime = currentMillis;
  }
}

void controlPH() {
  if (sensorError || isnan(currentPH)) return;
  // 3. pH dose lockout
  if (millis() - lastPHAction < PH_DOSE_LOCKOUT_MS) return;

  float error = currentPH - phTarget;
  if (abs(error) < PH_HYSTERESIS) return;

  // Adaptive dose: 100ms per 0.1 pH error, max 1000ms
  unsigned long dose = constrain(abs(error) * 1000, 100, 1000);

  if (error > 0) { // Too basic, need Acid
    if (!acidPump.active) {
        startPump(acidPump, dose);
        lastPHAction = millis();
    }
  } else { // Too acidic, need Base
    if (!basePump.active) {
        startPump(basePump, dose);
        lastPHAction = millis();
    }
  }
}

void controlTemp() {
  if (sensorError) return;
  static bool heaterState = false;

  // 10. Temperature Hysteresis (P2)
  if (heaterState) {
    if (currentTemp > tempTarget + TEMP_HYSTERESIS) heaterState = false;
  } else {
    if (currentTemp < tempTarget - TEMP_HYSTERESIS) heaterState = true;
  }
  writeActuator(HEATER_PIN, heaterState);
}

void controlFeeding() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastFeedingTime > (unsigned long)feedingInterval) {
    startPump(nutrientPump, FEEDING_DURATION_MS);
    lastFeedingTime = currentMillis;
  }
}

void logData() {
  if (sensorError) return;

  // 13. Log LittleFS rotation (512KB limit)
  if (LittleFS.exists("/log.csv")) {
    File f = LittleFS.open("/log.csv", "r");
    size_t size = f.size();
    f.close();
    if (size > 512 * 1024) {
      Serial.println("Log rotation: /log.csv > 512KB, renaming to /log.old.csv");
      if (LittleFS.exists("/log.old.csv")) LittleFS.remove("/log.old.csv");
      LittleFS.rename("/log.csv", "/log.old.csv");
    }
  }

  File file = LittleFS.open("/log.csv", "a");
  if (file) {
    DateTime now = rtc.now();
    // 13. Verify write return
    int bytesWritten = file.printf("%s,%.2f,%.2f,%.2f,%.2f\n",
                                  now.timestamp().c_str(), currentPH, currentOD, currentTemp, growthRate);
    if (bytesWritten <= 0) {
        Serial.println("Log write failed (disk full?)");
    }
    file.close();
  }
}

void sendLinkMessage(JsonDocument &payload) {
  payload["s"] = ++sequenceNum; // 9. Seq inside payload
  String body;
  serializeJson(payload, body);

  // 8. HH:<payloadJson>\n framing
  uint8_t crc = crc8((uint8_t*)body.c_str(), body.length());
  LinkSerial.printf("%02X:%s\n", crc, body.c_str());
}

void sendTelemetry() {
  JsonDocument doc;
  doc["type"] = "TELE";
  doc["ph"] = currentPH;
  doc["ph_v"] = currentPH_V;
  doc["od"] = currentOD;
  doc["temp"] = currentTemp;
  doc["mu"] = growthRate;
  doc["uv"] = currentUV_V;
  doc["pres"] = currentPressure_V;
  doc["p_a"] = acidPump.active;
  doc["p_b"] = basePump.active;
  doc["p_n"] = nutrientPump.active;
  doc["heat"] = digitalRead(HEATER_PIN);
  doc["stir"] = stirrerSpeed;
  doc["err"] = sensorError;
  doc["lost"] = linkLost;
  sendLinkMessage(doc);
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
            Serial.printf("Link gap: %u -> %u\n", lastReceivedSeq, seq);
          }
          lastReceivedSeq = seq;

          lastValidLinkMessage = millis();
          linkLost = false;

          String type = m["type"];
          if (type == "HB") {
            long epoch = m["epoch"];
            if (epoch > 0) rtc.adjust(DateTime(epoch));
          } else { // 17. Use 'else' (replaces redundant 'else if')
            if (type == "SET_T") {
              float ph = m["ph"];
              float t = m["temp"];
              if (ph >= PH_MIN && ph <= PH_MAX) phTarget = ph;
              if (t >= TEMP_MIN && t <= TEMP_MAX) tempTarget = t;
            } else if (type == "CAL") {
              // 1. Validation CAL
              float s = m["ph_s"];
              float o = m["ph_o"];
              float z = m["od_z"];
              if (isfinite(s) && abs(s) > 0.0001f && isfinite(o) && isfinite(z) && z > 0) {
                phSlope = s;
                phOffset = o;
                odZeroVoltage = z;
              } else {
                Serial.println("Rejected invalid CAL frame");
              }
            } else if (type == "PUMP") {
              String p = m["pump"];
              int dur = m["dur"];
              if (p == "acid") startPump(acidPump, dur);
              else if (p == "base") startPump(basePump, dur);
              else if (p == "nutrient") startPump(nutrientPump, dur);
            }
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
    linkLost = true;
  }
}

void setup() {
  // 4. Safe state at boot (P0) - MOVED TO TOP
  pinMode(PUMP_ACID_PIN, OUTPUT);
  pinMode(PUMP_BASE_PIN, OUTPUT);
  pinMode(PUMP_NUTRIENT_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  emergencyStop();

  Serial.begin(115200);

  // 5. Watchdog: 8 seconds timeout
  esp_task_wdt_init(8, true);
  if (esp_task_wdt_add(NULL) != ESP_OK) {
      Serial.println("WDT add warning");
  }

  LinkSerial.begin(115200, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);

  // Initialize other pins
  pinMode(OD_LIGHT_PIN, OUTPUT);
  pinMode(STIRRER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  if (!LittleFS.begin(true)) Serial.println("LittleFS Mount Failed");

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!ads.begin()) {
    sensorError = true;
  } else {
    ads.setGain(ADS_GAIN);
  }
  if (!rtc.begin()) sensorError = true;

  sensors.begin();
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
}

void loop() {
  esp_task_wdt_reset();
  unsigned long now = millis();

  handleLink();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    updateSensors();
    lastSensorRead = now;
    if (!sensorError) {
      controlPH();
      controlTemp();
      controlFeeding();
    }
  }

  updatePumps();

  if (now - lastLogTime >= LOG_INTERVAL_MS) {
    logData();
    lastLogTime = now;
  }

  if (now - lastTelemetrySend >= TELEMETRY_INTERVAL_MS) {
    sendTelemetry();
    lastTelemetrySend = now;
  }

  if (now - lastHeartbeatSend >= HEARTBEAT_INTERVAL_MS) {
    JsonDocument doc;
    doc["type"] = "HB";
    doc["uptime"] = now / 1000;
    sendLinkMessage(doc);
    lastHeartbeatSend = now;
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }

  if (!sensorError) {
    analogWrite(STIRRER_PIN, stirrerSpeed);
  } else {
    analogWrite(STIRRER_PIN, 0);
  }

  // Ensure idle task gets time to prevent WDT trigger in normal operation
  delay(1);
}
