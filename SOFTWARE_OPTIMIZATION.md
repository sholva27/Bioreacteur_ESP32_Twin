# Software Optimization

## Brain A (Control)
- **Interrupts**: Use hardware interrupts for the flow sensors (if added) to ensure zero missed pulses.
- **Fixed-Point Math**: Convert pH calculations to fixed-point if float performance becomes a bottleneck (unlikely on ESP32-S3 but good practice).
- **Task Scheduling**: Use FreeRTOS tasks to separate "Link Management", "Sensor Sampling", and "Actuator Control".

## Brain B (Connectivity)
- **Asynchronous Everything**: Continue using `ESPAsyncWebServer` to prevent network latency from blocking the main loop.
- **JSON Optimization**: Use `StaticJsonDocument` where possible to avoid heap fragmentation.
- **Payload Compression**: Consider binary framing (Protobuf or MsgPack) instead of JSON for the inter-brain link if telemetry frequency increases beyond 10Hz.

## General
- **Watchdog Timer**: Enable Task Watchdog (TWDT) on both brains to auto-recover from rare lockups.
- **Memory**: Monitor heap usage regularly, especially on Brain B when multiple clients are connected to the dashboard.
