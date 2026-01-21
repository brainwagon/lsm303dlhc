#include <stdio.h>
#include <stdlib.h>
#include "lsm303dlhc.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "lsm303dlhc";

// Accelerometer Registers
#define LSM303_CTRL_REG1_A       0x20
#define LSM303_CTRL_REG3_A       0x22
#define LSM303_CTRL_REG4_A       0x23
#define LSM303_CTRL_REG6_A       0x25
#define LSM303_OUT_X_L_A         0x28

#define LSM303_INT1_CFG_A        0x30
#define LSM303_INT1_SRC_A        0x31
#define LSM303_INT1_THS_A        0x32
#define LSM303_INT1_DURATION_A   0x33

#define LSM303_INT2_CFG_A        0x34
#define LSM303_INT2_SRC_A        0x35
#define LSM303_INT2_THS_A        0x36
#define LSM303_INT2_DURATION_A   0x37

// Magnetometer Registers
#define LSM303_CRA_REG_M         0x00
#define LSM303_CRB_REG_M         0x01
#define LSM303_MR_REG_M          0x02
#define LSM303_OUT_X_H_M         0x03

// Auto-increment bit for Accel (not always needed for Mag, varies by specific ST sensor generation, but usually implicit or MSB)
// Datasheet page 19: "7 LSBs represent the actual register address while the MSB enables address auto-increment."
#define ACCEL_AUTO_INCR          0x80

struct lsm303dlhc_dev_t {
    i2c_port_t port;
    lsm303dlhc_config_t config;
};

static esp_err_t write_reg(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(port, addr, write_buf, sizeof(write_buf), pdMS_TO_TICKS(100));
}

static esp_err_t read_reg(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *val) {
    return i2c_master_write_read_device(port, addr, &reg, 1, val, 1, pdMS_TO_TICKS(100));
}

esp_err_t lsm303dlhc_init(const lsm303dlhc_config_t *config, lsm303dlhc_handle_t *ret_handle) {
    if (config == NULL || ret_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    struct lsm303dlhc_dev_t *dev = calloc(1, sizeof(struct lsm303dlhc_dev_t));
    if (dev == NULL) {
        return ESP_ERR_NO_MEM;
    }
    dev->port = config->i2c_port;
    dev->config = *config;

    esp_err_t ret;

    // --- Accelerometer Setup ---
    // CTRL_REG1_A: ODR[3:0], LPen, Zen, Yen, Xen
    // Enable X, Y, Z (0x07) | ODR shifted
    uint8_t ctrl1_a = (config->accel_odr << 4) | 0x07;
    ret = write_reg(dev->port, LSM303DLHC_ACCEL_ADDR, LSM303_CTRL_REG1_A, ctrl1_a);
    if (ret != ESP_OK) goto err;

    // CTRL_REG4_A: BDU, BLE, FS[1:0], HR, 0, 0, SIM
    // BDU=1 (Block update), HR=1 (High Res), FS from config
    uint8_t ctrl4_a = 0x88 | (config->accel_fs << 4);
    ret = write_reg(dev->port, LSM303DLHC_ACCEL_ADDR, LSM303_CTRL_REG4_A, ctrl4_a);
    if (ret != ESP_OK) goto err;

    // --- Magnetometer Setup ---
    // CRA_REG_M: TempEnable(0), 0, 0, DO[2:0] (Data Rate), 0, 0
    uint8_t cra_m = (config->mag_odr << 2);
    ret = write_reg(dev->port, LSM303DLHC_MAG_ADDR, LSM303_CRA_REG_M, cra_m);
    if (ret != ESP_OK) goto err;

    // CRB_REG_M: GN[2:0] (Gain), 0, 0, 0, 0, 0
    uint8_t crb_m = (config->mag_gain << 5);
    ret = write_reg(dev->port, LSM303DLHC_MAG_ADDR, LSM303_CRB_REG_M, crb_m);
    if (ret != ESP_OK) goto err;

    // MR_REG_M: 0,0,0,0,0,0, MD[1:0] (Mode)
    // Continuous conversion = 00
    ret = write_reg(dev->port, LSM303DLHC_MAG_ADDR, LSM303_MR_REG_M, 0x00);
    if (ret != ESP_OK) goto err;

    *ret_handle = dev;
    ESP_LOGI(TAG, "LSM303DLHC Initialized");
    return ESP_OK;

err:
    free(dev);
    ESP_LOGE(TAG, "Initialization failed");
    return ret;
}

esp_err_t lsm303dlhc_deinit(lsm303dlhc_handle_t handle) {
    if (handle) {
        free(handle);
    }
    return ESP_OK;
}

esp_err_t lsm303dlhc_read_accel(lsm303dlhc_handle_t handle, lsm303dlhc_raw_data_t *data) {
    if (!handle || !data) return ESP_ERR_INVALID_ARG;

    uint8_t reg = LSM303_OUT_X_L_A | ACCEL_AUTO_INCR;
    uint8_t buf[6];
    
    // Read 6 bytes
    esp_err_t ret = i2c_master_write_read_device(handle->port, LSM303DLHC_ACCEL_ADDR, 
                                                 &reg, 1, buf, 6, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) return ret;

    // Little Endian: Low byte at lower address
    data->x = (int16_t)((buf[1] << 8) | buf[0]);
    data->y = (int16_t)((buf[3] << 8) | buf[2]);
    data->z = (int16_t)((buf[5] << 8) | buf[4]);

    return ESP_OK;
}

esp_err_t lsm303dlhc_read_mag(lsm303dlhc_handle_t handle, lsm303dlhc_raw_data_t *data) {
    if (!handle || !data) return ESP_ERR_INVALID_ARG;

    uint8_t reg = LSM303_OUT_X_H_M; // Mag usually increments automatically for data read
    uint8_t buf[6];

    esp_err_t ret = i2c_master_write_read_device(handle->port, LSM303DLHC_MAG_ADDR,
                                                 &reg, 1, buf, 6, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) return ret;

    // Big Endian for Mag: High byte at lower address
    // Order from register map: XH, XL, ZH, ZL, YH, YL
    data->x = (int16_t)((buf[0] << 8) | buf[1]);
    data->z = (int16_t)((buf[2] << 8) | buf[3]);
    data->y = (int16_t)((buf[4] << 8) | buf[5]);

    return ESP_OK;
}

esp_err_t lsm303dlhc_config_int1_pin(lsm303dlhc_handle_t handle, const lsm303dlhc_int1_pin_conf_t *conf) {
    if (!handle || !conf) return ESP_ERR_INVALID_ARG;

    // CTRL_REG3_A bits: I1_CLICK, I1_AOI1, I1_AOI2, I1_DRDY1, I1_DRDY2, I1_WTM, I1_OVERRUN, --
    uint8_t reg_val = 0;
    if (conf->i1_click)   reg_val |= (1 << 7);
    if (conf->i1_aoi1)    reg_val |= (1 << 6);
    if (conf->i1_aoi2)    reg_val |= (1 << 5);
    if (conf->i1_drdy1)   reg_val |= (1 << 4);
    if (conf->i1_drdy2)   reg_val |= (1 << 3);
    if (conf->i1_wtm)     reg_val |= (1 << 2);
    if (conf->i1_overrun) reg_val |= (1 << 1);

    return write_reg(handle->port, LSM303DLHC_ACCEL_ADDR, LSM303_CTRL_REG3_A, reg_val);
}

esp_err_t lsm303dlhc_config_int2_pin(lsm303dlhc_handle_t handle, const lsm303dlhc_int2_pin_conf_t *conf) {
    if (!handle || !conf) return ESP_ERR_INVALID_ARG;

    // CTRL_REG6_A bits: I2_CLICKen, I2_INT1, I2_INT2, BOOT_I1, P2_ACT, --, H_LACTIVE, --
    uint8_t reg_val = 0;
    if (conf->i2_click)   reg_val |= (1 << 7);
    if (conf->i2_int1)    reg_val |= (1 << 6);
    if (conf->i2_int2)    reg_val |= (1 << 5);
    if (conf->boot_i1)    reg_val |= (1 << 4);
    if (conf->p2_act)     reg_val |= (1 << 3);
    if (conf->h_lactive)  reg_val |= (1 << 1);

    return write_reg(handle->port, LSM303DLHC_ACCEL_ADDR, LSM303_CTRL_REG6_A, reg_val);
}

static esp_err_t config_int_event_gen(lsm303dlhc_handle_t handle, const lsm303dlhc_int_event_conf_t *conf, 
                                      uint8_t cfg_reg, uint8_t ths_reg, uint8_t dur_reg) {
    if (!handle || !conf) return ESP_ERR_INVALID_ARG;

    // CFG bits: AOI, 6D, ZHIE, ZLIE, YHIE, YLIE, XHIE, XLIE
    uint8_t cfg_val = 0;
    if (conf->aoi)   cfg_val |= (1 << 7);
    if (conf->six_d) cfg_val |= (1 << 6);
    if (conf->zhie)  cfg_val |= (1 << 5);
    if (conf->zlie)  cfg_val |= (1 << 4);
    if (conf->yhie)  cfg_val |= (1 << 3);
    if (conf->ylie)  cfg_val |= (1 << 2);
    if (conf->xhie)  cfg_val |= (1 << 1);
    if (conf->xlie)  cfg_val |= (1 << 0);

    // Write Threshold
    esp_err_t ret = write_reg(handle->port, LSM303DLHC_ACCEL_ADDR, ths_reg, conf->threshold & 0x7F);
    if (ret != ESP_OK) return ret;

    // Write Duration
    ret = write_reg(handle->port, LSM303DLHC_ACCEL_ADDR, dur_reg, conf->duration & 0x7F);
    if (ret != ESP_OK) return ret;

    // Write Configuration (this enables the generator)
    return write_reg(handle->port, LSM303DLHC_ACCEL_ADDR, cfg_reg, cfg_val);
}

esp_err_t lsm303dlhc_config_int1_event(lsm303dlhc_handle_t handle, const lsm303dlhc_int_event_conf_t *conf) {
    return config_int_event_gen(handle, conf, LSM303_INT1_CFG_A, LSM303_INT1_THS_A, LSM303_INT1_DURATION_A);
}

esp_err_t lsm303dlhc_config_int2_event(lsm303dlhc_handle_t handle, const lsm303dlhc_int_event_conf_t *conf) {
    return config_int_event_gen(handle, conf, LSM303_INT2_CFG_A, LSM303_INT2_THS_A, LSM303_INT2_DURATION_A);
}

esp_err_t lsm303dlhc_get_int1_src(lsm303dlhc_handle_t handle, uint8_t *src) {
    if (!handle || !src) return ESP_ERR_INVALID_ARG;
    return read_reg(handle->port, LSM303DLHC_ACCEL_ADDR, LSM303_INT1_SRC_A, src);
}

esp_err_t lsm303dlhc_get_int2_src(lsm303dlhc_handle_t handle, uint8_t *src) {
    if (!handle || !src) return ESP_ERR_INVALID_ARG;
    return read_reg(handle->port, LSM303DLHC_ACCEL_ADDR, LSM303_INT2_SRC_A, src);
}

