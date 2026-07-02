# Hardware Optimization

## Power Supply
- Use a high-quality 12V 5A DC power supply.
- Use separate 3.3V LDOs or Buck converters for Brain A and Brain B to prevent noise from WiFi (Brain B) affecting the ADC (Brain A).
- Add 1000uF capacitors near the pump drivers to absorb voltage spikes.

## Signal Integrity
- Keep I2C lines (SDA/SCL) short. Use 4.7k pull-up resistors.
- Shield the pH probe cable. The impedance is very high, making it susceptible to EMI from the stirrer motor.
- Use an optoisolator for the inter-brain UART link if the boards are powered from different sources.

## Actuators
- **Pumps**: Use peristaltic pumps with silicone tubing for chemical resistance.
- **Drivers**: Use MOSFETs (e.g., IRLZ44N) with flyback diodes (1N4007) for all inductive loads.
- **Heater**: Use a 12V 50W cartridge heater inside a stainless steel thermowell.
