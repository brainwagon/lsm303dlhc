# LSM303DLHC Test Example

This example demonstrates how to initialize and read raw data from the LSM303DLHC accelerometer and magnetometer.

## Understanding Sensor Units

The driver provides raw data as 16-bit signed integers (`int16_t`). Here is how to interpret the output values:

### Accelerometer
*   **Scale:** Default is ±2g.
*   **Resolution:** 12-bit (left-aligned in the 16-bit word).
*   **Sensitivity:** At ±2g, sensitivity is ~1 mg/LSB (on the 12-bit value).
*   **Conversion:** Because the 12-bit value is left-shifted by 4 bits, a value of **16384** represents approximately **1g** (9.8 m/s²).

### Magnetometer
*   **Resolution:** 12-bit.
*   **Sensitivity:** Depends on the Gain setting. 
    *   At default **Gain 1.3** (GN=001):
        *   X/Y Axis: 1100 LSB/Gauss
        *   Z Axis: 980 LSB/Gauss
*   **Overflow Indicator (-4096):** 
    If you see a value of **-4096** (0xF000), it indicates an **ADC overflow**. This means the magnetic field is too strong for the current gain setting on that axis. To fix this, increase the measurement range by choosing a higher gain setting (e.g., `LSM303DLHC_MAG_GAIN_4_0`).

## How to use
1. Connect the LSM303DLHC to your ESP32:
   - SCL: GPIO 22
   - SDA: GPIO 21
   - VCC: 3.3V
   - GND: GND
2. Build and flash the example:
   ```bash
   idf.py build flash monitor
   ```
