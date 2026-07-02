# Debugging and Failure-Mode Testing

## Failure-Mode Test Plan

### 1. UART Link Loss
- **Action**: Unplug the RX/TX wires between Brain A and Brain B while the system is running.
- **Expected Brain A Behavior**: Control loop continues (temp/pH regulation); `linkLost` flag becomes `true` in logs.
- **Expected Brain B Behavior**: Dashboard shows "CONTROL NODE OFFLINE"; MQTT publishes offline status.

### 2. Brain B Power Cycle
- **Action**: Power cycle Brain B while Brain A is regulating.
- **Expected Brain A Behavior**: No glitching of pumps or heater; continues regulation autonomously.
- **Expected Brain B Behavior**: Recovers link after boot; dashboard resumes showing live data.

### 3. Sensor Disconnect (Brain A)
- **Action**: Unplug the I2C line or ADS1115 from Brain A.
- **Expected Behavior**: Brain A enters `emergencyStop()` immediately; all actuators OFF; `err` flag sent to Brain B.

### 4. Command Validation
- **Action**: Use the dashboard or MQTT to send an out-of-bounds pH target (e.g., pH 1.0).
- **Expected Behavior**: Brain A receives the command but refuses to update its internal setpoint because it's outside the hard-coded safe range (PH_MIN/PH_MAX).

## Advanced Debugging

### UART Link Monitoring
Both brains log link events to their respective hardware Serial (USB) ports.
- **"Link gap detected: X -> Y"**: Indicates dropped messages. Check wiring and EMI.
- **"Link CRC error!"**: Indicates corrupted messages.
- **"LittleFS full, deleting old log"**: Brain A automatically rotates the local log if space is low.

### Watchdog Timer (WDT)
If Brain A's `loop()` hangs for more than 5 seconds, the `esp_task_wdt` will trigger a hardware reset, putting all pumps into a safe state (defined by hardware pull-downs and the initial `emergencyStop()` in `setup()`).

## Common Issues
- **Check CRC Errors**: If CRC errors occur frequently, check for GND connection between boards and keep UART wires short.
- **Baud Rate Mismatch**: Ensure both brains are set to 115200.
