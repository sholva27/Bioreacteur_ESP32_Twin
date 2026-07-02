# Bioreacteur ESP32-S3 Dual-Brain Hardware

This document describes the hardware architecture for the dual-brain Bioreacteur.

## Overview
The system uses two ESP32-S3-WROOM-1 boards to separate safety-critical control logic from network-facing connectivity logic.

- **Brain A (Control Node)**: Handles sensors, actuators, and the PID control loop. No WiFi/Bluetooth enabled.
- **Brain B (Connectivity Node)**: Handles WiFi, Web Dashboard, MQTT, and OTA updates.

## Wiring Diagram (Inter-Brain Link)

| Brain A (Control) | Brain B (Connectivity) | Function |
|-------------------|------------------------|----------|
| GPIO 1 (TX1)      | GPIO 2 (RX1)           | Data A -> B |
| GPIO 2 (RX1)      | GPIO 1 (TX1)           | Data B -> A |
| GND               | GND                    | Common Ground |
| 3.3V              | 3.3V                   | Power (Optional) |

## Brain A Pinout (Actuators & Sensors)

| Pin | Function |
|-----|----------|
| 8   | I2C SDA (ADS1115, RTC) |
| 9   | I2C SCL (ADS1115, RTC) |
| 10  | Acid Pump |
| 11  | Base Pump |
| 12  | Nutrient Pump |
| 13  | OD Light (LED) |
| 14  | OneWire (DS18B20 Temp) |
| 15  | Heater Control |
| 16  | Stirrer Control (PWM) |
| 17  | Manual Touch Button |
| 18  | Fluorescence LED |
| 21  | Status LED |

## Bill of Materials (BOM) Addition
- 2x ESP32-S3-WROOM-1 Development Boards.
- 4x Jumper wires (or soldered wires) for the inter-brain link.
