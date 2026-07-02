# System Schematic & Wiring

## Logic Power
- **Input**: 12V DC.
- **5V Rail**: From Buck Converter. Powers ADS1115 and RTC.
- **3.3V Rail**: From ESP32-S3 onboard regulators. Powers internal logic and DS18B20.

## Wiring Table

| Component | Pin (Brain A) | Notes |
|-----------|---------------|-------|
| ADS1115 SDA | GPIO 8 | 4.7k Pull-up to 3.3V |
| ADS1115 SCL | GPIO 9 | 4.7k Pull-up to 3.3V |
| DS18B20 DQ | GPIO 14 | 4.7k Pull-up to 3.3V |
| Acid Pump | GPIO 10 | To MOSFET Gate |
| Base Pump | GPIO 11 | To MOSFET Gate |
| Nutri Pump | GPIO 12 | To MOSFET Gate |
| Heater | GPIO 15 | To MOSFET Gate |
| Stirrer PWM | GPIO 16 | To MOSFET Gate |
| Status LED | GPIO 21 | Via 220R Resistor |

## Inter-Brain Link
| Pin (Brain A) | Pin (Brain B) | Function |
|---------------|---------------|----------|
| GPIO 1 (TX)   | GPIO 2 (RX)   | Link A->B |
| GPIO 2 (RX)   | GPIO 1 (TX)   | Link B->A |
| GND           | GND           | Signal GND |

## ADC Mapping (ADS1115)
- **A0**: pH Sensor Signal (0-5V or buffered).
- **A1**: OD Photodiode.
- **A2**: Headspace Pressure Sensor.
- **A3**: UV Sensor.
