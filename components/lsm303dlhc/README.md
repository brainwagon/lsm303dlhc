# LSM303DLHC Driver for ESP-IDF

> **NOTE:** The basic functionality of this module (reading accelerometer and magnetometer data) has been tested on physical hardware and appears to function as expected. Interrupt processing has not yet been tested. This was an experiment by Mark VandeWettering <mvandewettering@gmail.com> in generating ESP-IDF drivers purely using the `gemini-cli`.

## Features

*   **I2C Communication**: Handles the split I2C addresses (0x19 for Accel, 0x1E for Mag).
*   **Endianness Handling**: Automatically handles Little Endian (Accel) and Big Endian (Mag) data.
*   **Flexible Configuration**:
    *   Selectable Full Scale (2g, 4g, 8g, 16g).
    *   Configurable Output Data Rates (ODR).
    *   Magnetometer Gain settings.
*   **Interrupt Support**:
    *   Full configuration of INT1 and INT2 pins.
    *   Support for Wake-up, Free-fall, and 6D orientation events.

## Installation

Copy this directory into the `components` directory of your ESP-IDF project.

## Usage

### Initialization

```c
#include "lsm303dlhc.h"

// ... inside app_main ...

lsm303dlhc_config_t config = {
    .i2c_port = I2C_NUM_0, // Ensure I2C driver is installed
    .accel_fs = LSM303DLHC_ACCEL_FS_2G,
    .accel_odr = LSM303DLHC_ACCEL_ODR_10HZ,
    .mag_odr = LSM303DLHC_MAG_ODR_7_5HZ,
    .mag_gain = LSM303DLHC_MAG_GAIN_1_3
};

lsm303dlhc_handle_t sensor;
esp_err_t ret = lsm303dlhc_init(&config, &sensor);
if (ret != ESP_OK) {
    // Handle error
}
```

### Reading Data

```c
lsm303dlhc_raw_data_t accel_data;
lsm303dlhc_raw_data_t mag_data;

lsm303dlhc_read_accel(sensor, &accel_data);
lsm303dlhc_read_mag(sensor, &mag_data);

printf("Accel: X=%d Y=%d Z=%d\n", accel_data.x, accel_data.y, accel_data.z);
```

### Interrupt Configuration (Example: Wake on Motion)

```c
// Configure the event generator to trigger on High X, Y, or Z movement
lsm303dlhc_int_event_conf_t event_conf = {
    .aoi = false,      // OR combination
    .xhie = true, .yhie = true, .zhie = true,
    .threshold = 10,   // Sensitivity threshold
    .duration = 0      // Trigger immediately
};
lsm303dlhc_config_int1_event(sensor, &event_conf);

// Route the internal interrupt signal to the physical INT1 pin
lsm303dlhc_int1_pin_conf_t pin_conf = {
    .i1_aoi1 = true
};
lsm303dlhc_config_int1_pin(sensor, &pin_conf);
```

