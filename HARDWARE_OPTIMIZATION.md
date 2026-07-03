# Hardware Optimization

## 1. Independent Emergency Stop
**Critical**: Install a physical punch-style emergency stop button wired directly to the power supply rail of the pump and heater drivers. This ensures safety even in the event of a total firmware lockup where GPIO control is lost.

## 2. Gravimetric Dosing
Replace time-based dosing with real-time volume verification. By placing load cells under the acid, base, and nutrient reservoirs, the system can detect clogs, empty bottles, and pump wear by measuring the actual mass of liquid delivered.

## 3. Sensor Redundancy
Add a second DS18B20 temperature sensor for cross-redundancy. If the deviation between sensors exceeds a threshold, the system should trigger a fault and enter a safe state, preventing overheating due to a single-sensor failure.

## 4. Galvanic Isolation (Optocouplers)
Use optocouplers for all actuator signals (Pumps, Heater) and not just the communication link. This provides total galvanic isolation between the low-voltage control logic and the higher-power actuator rails, critical for safety in liquid-handling environments.

## 5. I2C Multiplexing (TCA9548A)
Integrate an I2C multiplexer to separate the sensitive pH probe bus from future I2C sensors (DO, AS7341 spectral sensor). This prevents address conflicts and reduces cross-talk noise between devices.

## 6. Power & Integrity
- Use separate 3.3V LDOs for Brain A and Brain B.
- Use 4.7k pull-up resistors on all I2C lines.
- Shield the pH probe cable from motor EMI.
