#ifndef CONFIG_H
#define CONFIG_H

// Brain A - Control Node specific config

// Pin Assignments (ESP32-S3)
#define I2C_SDA 8
#define I2C_SCL 9

#define PUMP_ACID_PIN 10
#define PUMP_BASE_PIN 11
#define PUMP_NUTRIENT_PIN 12
#define OD_LIGHT_PIN 13
#define TEMP_SENSOR_PIN 14
#define HEATER_PIN 15
#define STIRRER_PIN 16
#define STATUS_LED 21

// UART for Inter-brain Link
#define LINK_TX_PIN 1
#define LINK_RX_PIN 2

// ADS1115
#define ADS_PH_CH 0
#define ADS_OD_CH 1
#define ADS_PRESSURE_CH 2
#define ADS_UV_CH 3
#define ADS_GAIN GAIN_ONE
#define ADS_LSB 0.000125f

// pH Sensor Midpoint and Slope (Nernstian)
// Note: Slope must be calibrated at the target service temperature (e.g. 37°C)
#define PH_VMID 2.5          // Voltage at pH 7.0

// System Constants
#define PH_TARGET_DEFAULT 7.0
#define PH_HYSTERESIS 0.1
#define TEMP_TARGET_DEFAULT 37.0
#define TEMP_HYSTERESIS 0.5
#define STIRRER_SPEED_DEFAULT 128
#define FEEDING_INTERVAL_MS_DEFAULT 3600000 // 1 hour
#define FEEDING_DURATION_MS 5000     // 5 seconds
#define PUMP_MAX_MS 10000            // 10 seconds safety cap
#define OD_CALIBRATION_FACTOR 1.0
#define SENSOR_READ_INTERVAL_MS 2000
#define LOG_INTERVAL_MS 60000        // 1 minute

#define PH_DOSE_LOCKOUT_MS 30000     // 30s mixing time after dose

#define LINK_TIMEOUT_MS 30000
#define TELEMETRY_INTERVAL_MS 2000
#define HEARTBEAT_INTERVAL_MS 5000

// Hard Limits for Validation
#define PH_MIN 4.0
#define PH_MAX 10.0
#define TEMP_MIN 20.0
#define TEMP_MAX 45.0

// Actuator Polarity
#define RELAY_ACTIVE_LOW 0  // Set to 1 if using Active-Low relays

#endif
