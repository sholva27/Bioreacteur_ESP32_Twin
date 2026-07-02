#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Brain B - Connectivity Node specific config

// UART for Inter-brain Link
#define LINK_TX_PIN 1
#define LINK_RX_PIN 2

#define STATUS_LED 21

// MQTT Settings
#define MQTT_BROKER "192.168.1.50"
#define MQTT_PORT 1883
#define MQTT_TOPIC_PREFIX "biofermenter/1"

#define LINK_TIMEOUT_MS 30000
#define HEARTBEAT_INTERVAL_MS 5000

#endif
