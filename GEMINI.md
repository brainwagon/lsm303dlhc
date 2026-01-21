# LSM303DLHC ESP-IDF Component

## Project Overview

This project is an **ESP-IDF component** driver for the STMicroelectronics **LSM303DLHC** Ultra-compact high-performance e-compass, 3D accelerometer, and 3D magnetometer module.

The component is designed to be dropped into the `components/` directory of an ESP-IDF project. It handles the I2C communication specifics (including differing endianness between Accel and Mag sensors) and provides a clean C API for initialization, configuration, and reading sensor data.

### Key Features
*   **Initialization:** Configures both Accelerometer (0x19) and Magnetometer (0x1E).
*   **Data Reading:**
    *   **Accelerometer:** Little-endian parsing.
    *   **Magnetometer:** Big-endian parsing.
*   **Configuration:**
    *   Output Data Rates (ODR) for both sensors.
    *   Full-scale selection (2g, 4g, 8g, 16g).
    *   Magnetometer Gain configuration.
*   **Interrupt Support:**
    *   Pin routing configuration for `INT1` and `INT2` pins.
    *   Event generator configuration (Thresholds, Duration, 6D orientation, Wake-up events).
    *   Interrupt source reading.

## Directory Structure

```
/home/markv/lsm303dlhc/
├── components/
│   └── lsm303dlhc/
│       ├── CMakeLists.txt      # ESP-IDF build configuration
│       ├── lsm303dlhc.c        # Implementation source
│       └── include/
│           └── lsm303dlhc.h    # Public API header
├── LSM303DLHC.PDF              # Datasheet
└── GEMINI.md                   # Context file (this file)
```

## Usage

To use this component in your ESP-IDF project:

1.  **Installation:** Ensure the `components/lsm303dlhc` folder is present in your project's `components` directory.
2.  **Configuration:** The component requires the I2C driver to be initialized in your application (typically in `app_main`).

### Example Code

```c
#include "lsm303dlhc.h"
#include "driver/i2c.h"

// ... I2C Initialization ...

void app_main(void) {
    // 1. Initialize Configuration
    lsm303dlhc_config_t config = {
        .i2c_port = I2C_NUM_0,
        .accel_fs = LSM303DLHC_ACCEL_FS_2G,
        .accel_odr = LSM303DLHC_ACCEL_ODR_100HZ,
        .mag_odr = LSM303DLHC_MAG_ODR_15HZ,
        .mag_gain = LSM303DLHC_MAG_GAIN_1_3
    };

    lsm303dlhc_handle_t sensor;
    if (lsm303dlhc_init(&config, &sensor) == ESP_OK) {
        
        // 2. Configure Interrupts (Optional)
        // Example: Wake-up on motion
        lsm303dlhc_int_event_conf_t event_conf = {
            .aoi = false,      // OR combination
            .xhie = true, .yhie = true, .zhie = true,
            .threshold = 10,
            .duration = 0
        };
        lsm303dlhc_config_int1_event(sensor, &event_conf);
        
        lsm303dlhc_int1_pin_conf_t pin_conf = {
            .i1_aoi1 = true    // Route to INT1 pin
        };
        lsm303dlhc_config_int1_pin(sensor, &pin_conf);

        // 3. Read Data Loop
        lsm303dlhc_raw_data_t accel, mag;
        while (1) {
            lsm303dlhc_read_accel(sensor, &accel);
            lsm303dlhc_read_mag(sensor, &mag);
            printf("A: %d %d %d | M: %d %d %d\n", accel.x, accel.y, accel.z, mag.x, mag.y, mag.z);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
```

## Development Conventions

*   **API Design:** Handle-based C API (`lsm303dlhc_handle_t`).
*   **Error Handling:** All functions return `esp_err_t` to propagate errors (I2C failures, invalid arguments).
*   **Endianness:** The driver abstracts away the endianness differences between the sensors.
*   **Memory:** The `init` function allocates memory for the device handle, which should be freed with `deinit` if necessary.
