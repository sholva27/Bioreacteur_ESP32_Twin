# Project Roadmap

## Phase 1: Dual-Brain Stability (Current)
- [x] Non-blocking inter-brain UART link with CRC/Seq verification.
- [x] Persistent setpoints in NVS (Brain A).
- [x] Basic pH/Temp control loop with safety lockouts and hysteresis.
- [x] Web dashboard and MQTT integration (Brain B).

## Phase 2: Production Hardware & Safety
- [ ] Integration of physical Emergency Stop button.
- [ ] PCB design with galvanic isolation for all actuators.
- [ ] Gravimetric dosing (Load cells for reservoir bottles).
- [ ] Dual-sensor temperature redundancy.
- [ ] I2C Multiplexing for future sensor expansion.

## Phase 3: Advanced Analytics & Connectivity
- [ ] Sliding Gompertz fit for high-fidelity µ calculation.
- [ ] Direct database logging (InfluxDB/Postgres).
- [ ] Bridge-OTA for the Control Node (Brain A).
- [ ] Integration of AS7341 spectral sensor for metabolism monitoring.

## Phase 4: Intelligence & Parallelization
- [ ] Digital Twin: Real-time predictive growth vs. measure comparison.
- [ ] Multi-vessel bus architecture for parallel strain screening.
- [ ] Automated BATH/MATH laboratory assay protocols.
- [ ] Automated pH calibration using buffer auto-detection.
