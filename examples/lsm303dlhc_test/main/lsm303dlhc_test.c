#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "lsm303dlhc.h"

static const char *TAG = "LSM303_TEST";

// I2C Configuration
#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data */
#define I2C_MASTER_NUM              0       /*!< I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 
                              I2C_MASTER_RX_BUF_DISABLE, 
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C...");
    ESP_ERROR_CHECK(i2c_master_init());

    ESP_LOGI(TAG, "Initializing LSM303DLHC...");
    lsm303dlhc_config_t config = {
        .i2c_port = I2C_MASTER_NUM,
        .accel_fs = LSM303DLHC_ACCEL_FS_2G,
        .accel_odr = LSM303DLHC_ACCEL_ODR_50HZ,
        .mag_odr = LSM303DLHC_MAG_ODR_7_5HZ,
        .mag_gain = LSM303DLHC_MAG_GAIN_1_3
    };

    lsm303dlhc_handle_t sensor;
    if (lsm303dlhc_init(&config, &sensor) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return;
    }

    ESP_LOGI(TAG, "Sensor initialized. Reading data...");

    lsm303dlhc_raw_data_t accel;
    lsm303dlhc_raw_data_t mag;

    while (1) {
        // Read Accelerometer
        if (lsm303dlhc_read_accel(sensor, &accel) == ESP_OK) {
            ESP_LOGI(TAG, "ACCEL: X=%6d  Y=%6d  Z=%6d", accel.x, accel.y, accel.z);
        } else {
            ESP_LOGE(TAG, "Failed to read Accel");
        }

        // Read Magnetometer
        if (lsm303dlhc_read_mag(sensor, &mag) == ESP_OK) {
            ESP_LOGI(TAG, "MAG:   X=%6d  Y=%6d  Z=%6d", mag.x, mag.y, mag.z);
        } else {
            ESP_LOGE(TAG, "Failed to read Mag");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
