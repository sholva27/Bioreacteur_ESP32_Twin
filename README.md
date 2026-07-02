# Bioreacteur_ESP32_Twin

Split Bioreacteur firmware for dual-brain ESP32-S3 architecture.

## Architecture
- **firmware/brain-control**: Safety-critical control node (Sensors, Pumps, PID).
- **firmware/brain-connectivity**: Network-facing node (WiFi, MQTT, Web Dashboard).

## Documentation
- [PROTOCOL.md](PROTOCOL.md): UART communication protocol.
- [hardware.md](hardware.md): Wiring and pinout.
- [debugging.md](debugging.md): Test plan and failure modes.

## Build
```bash
pio run -e brain-control
pio run -e brain-connectivity
```
