# Bioreacteur ESP32-S3 Dual-Brain UART Protocol

This document defines the communication protocol between Brain A (Control Node) and Brain B (Connectivity Node).

## Physical Layer
- **Interface**: Hardware UART (UART1)
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None
- **Wiring**:
  - Brain A TX (GPIO 1) <-> Brain B RX (GPIO 2)
  - Brain A RX (GPIO 2) <-> Brain B TX (GPIO 1)
  - Brain A GND <-> Brain B GND
  - Common 3.3V power rail (if powered from same source)

## Framing & Serialization
- **Format**: JSON lines (one JSON object per line, terminated by `\n`).
- **Library**: ArduinoJson v7.
- **Envelope**: Every message is wrapped in an envelope containing a sequence number and a checksum.
  ```json
  {"s": 123, "c": 45, "m": { ...message body... }}
  ```
  - `s`: (int) Monotonically increasing sequence number.
  - `c`: (int) CRC8 checksum of the message body string.
  - `m`: (object) The actual payload.

## Logic Rules
- **Heartbeat**: Both brains send a `HEARTBEAT` every 5000ms.
- **Timeout**: If no valid message is received for `LINK_TIMEOUT_MS` (30000ms), the link is considered lost.
- **Brain A Behavior on Link Lost**:
  - Set `linkLost` flag to `true`.
  - Continue regulating with last known setpoints.
  - Refuse new commands from Brain B until a valid heartbeat/message is received.
- **Validation**: Brain A validates all incoming values (e.g., `phTarget` must be within `PH_MIN` and `PH_MAX`).
- **Checksum Failure**: If CRC8 does not match, the message is dropped and logged to Serial.
- **Framing**: Every message must end with a newline character (`\n`).

## Message Types (Payloads)

### Brain A -> Brain B (Control to Connectivity)

#### TELEMETRY
Sent every 2000ms.
```json
{
  "type": "TELE",
  "ph": 7.02,
  "ph_v": 2.48,
  "od": 0.15,
  "temp": 37.1,
  "mu": 0.02,
  "uv": 0.12,
  "pres": 1.01,
  "p_a": false,
  "p_b": false,
  "p_n": true,
  "heat": true,
  "stir": 128,
  "err": 0,
  "lost": false
}
```

#### HEARTBEAT
Sent every 5000ms.
```json
{
  "type": "HB",
  "uptime": 123456
}
```

### Brain B -> Brain A (Connectivity to Control)

#### HEARTBEAT / TIME_SYNC
Sent every 5000ms. Brain A uses this to sync its RTC if offset is large.
```json
{
  "type": "HB",
  "epoch": 1719934800
}
```

#### SET_TARGET
```json
{
  "type": "SET_T",
  "ph": 7.0,
  "temp": 37.0
}
```

#### CALIBRATE
```json
{
  "type": "CAL",
  "ph_s": 1.0,
  "ph_o": 0.0,
  "od_z": 2.1
}
```

#### MANUAL_PUMP
```json
{
  "type": "PUMP",
  "pump": "acid",
  "dur": 5000
}
```

#### GET_LOG
Requests the content of `/log.csv` from Brain A.
```json
{
  "type": "GET_LOG"
}
```
