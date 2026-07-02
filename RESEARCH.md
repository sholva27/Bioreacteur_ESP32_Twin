# Research & Development

## 1. pH Control Algorithms
- **Current**: Hysteresis-based pulse control (Simple).
- **Proposed**: Fuzzy logic or PID tuning for acidic/alkaline environments.
- **Reference**: Research shows that pH control in bioreactors is non-linear. The sensitivity increases as the pH approaches neutrality.

## 2. Optical Density (OD) Measurement
- **Method**: 600nm IR LED + Photodiode.
- **Challenges**: Biofouling on the probe. Proposed using magnetic wipers or ultrasonic cleaning.
- **Conversion**: Beer-Lambert law application for converting voltage to AU.

## 3. Dual-Brain Architecture Benefits
- **Isolation**: Keeping the TCP/IP stack away from real-time GPIO prevents "blocking" issues during WiFi reconnects.
- **Security**: Brain A has zero network surface area.
- **Redundancy**: Brain A can continue the mission even if Brain B is being flashed or crashed.

## 4. Fluorescence Sensing
- **NADH (Metabolic activity)**: 340nm excitation / 460nm emission.
- **Riboflavin**: 450nm excitation / 520nm emission.
- **Sensor Choice**: AS7341 spectral sensor for high-precision multi-channel monitoring.
