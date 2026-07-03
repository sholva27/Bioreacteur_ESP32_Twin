# Software Optimization

## 1. Advanced Growth Modeling (Gompertz Fit)
Replace naive exponential smoothing with a sliding-window Gompertz fit (YM, K, lag) to calculate the specific growth rate (µ). This aligns the real-time telemetry with standard laboratory research methodologies.

## 2. Formal State Machine (FSM)
Implement a robust FSM (INIT, RUNNING, FAULT, CALIBRATION, MAINTENANCE) to replace ad-hoc boolean flags like `sensorError`. This improves code maintainability and visibility into the bioreactor's current status.

## 3. Native Unit Testing (Unity)
Utilize the Unity framework for native (host-side) unit testing of critical logic components:
- CRC8 calculation and packet framing.
- pH and OD voltage-to-value conversion formulas.
- Gompertz fitting algorithms.

## 4. InfluxDB/Postgres Data Pipeline
Modify Brain B to push telemetry directly to a time-series database (InfluxDB) or relational database (Postgres). This allows for live data analysis using standard tools like Grafana, Pandas, or custom Python scripts without manual CSV exports.

## 5. Over-the-Air (OTA) for Control Node
Implement a bridge mechanism allowing Brain B to update Brain A's firmware over the inter-brain link, ensuring that the entire system can be updated remotely while maintaining architectural isolation.

## 6. Real-time digital twin
Simulate the expected culture growth curve on Brain B and trigger alerts if the measured OD deviates significantly from the prediction, indicating potential contamination or sensor failure.
