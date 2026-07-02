# Bioreacteur_ESP32_Twin

Split Bioreacteur firmware for dual-brain ESP32-S3 architecture.

## Architecture
- **firmware/brain-control**: Safety-critical control node (Sensors, Pumps, PID).
- **firmware/brain-connectivity**: Network-facing node (WiFi, MQTT, Web Dashboard).

## Documentation
- [PROTOCOL.md](PROTOCOL.md): UART communication protocol.
- [hardware.md](hardware.md): Wiring and pinout summary.
- [SCHEMATIC.md](SCHEMATIC.md): Detailed wiring and power.
- [BOM.md](BOM.md): Bill of materials.
- [debugging.md](debugging.md): Test plan and failure modes.
- [RESEARCH.md](RESEARCH.md): R&D background.
- [roadmap.md](roadmap.md): Future developments.
- [CREATIVE_PROPOSALS.md](CREATIVE_PROPOSALS.md): Innovation ideas.
- [HARDWARE_OPTIMIZATION.md](HARDWARE_OPTIMIZATION.md): Design tips.
- [SOFTWARE_OPTIMIZATION.md](SOFTWARE_OPTIMIZATION.md): Performance tips.

## Build
```bash
pio run -e brain-control
pio run -e brain-connectivity
```
