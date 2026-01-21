# ESP-IDF Driver for LSM303DLHC

> **WARNING:** This component was created as an experiment in generating ESP-IDF drivers purely using the `gemini-cli`. It has **not been tested** on physical hardware and is not guaranteed to be functional, accurate, or suitable for any purpose. This was an experiment by Mark VandeWettering <mvandewettering@gmail.com> and the code should be thoroughly reviewed and tested before any use.

This repository contains an ESP-IDF component for the **LSM303DLHC** e-compass module (3D Accelerometer + 3D Magnetometer).

## Repository Structure

*   **[components/lsm303dlhc](components/lsm303dlhc)**: The driver component itself. Copy this folder to your project's `components/` directory.
    *   See the [Component README](components/lsm303dlhc/README.md) for API documentation and usage details.
*   **[examples/lsm303dlhc_test](examples/lsm303dlhc_test)**: A standalone ESP-IDF example project demonstrating how to initialize the sensor and read data.
*   **LSM303DLHC.PDF**: The datasheet for the sensor.

## Quick Start

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/brainwagon/lsm303dlhc.git
    ```

2.  **Build the Example:**
    ```bash
    cd lsm303dlhc/examples/lsm303dlhc_test
    idf.py set-target esp32  # or esp32s3, esp32c3, etc.
    idf.py build
    idf.py flash monitor
    ```

## Features

*   **I2C Support**: Handles distinct addresses for Accel (0x19) and Mag (0x1E).
*   **Endianness**: Abstracts Little Endian (Accel) vs Big Endian (Mag) registers.
*   **Interrupts**: Full configuration for INT1/INT2 pins and event generators (Free-fall, Wake-up, 6D).

## License

See the [LICENSE](LICENSE) file for details.
