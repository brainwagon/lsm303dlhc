#pragma once

#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C Addresses
#define LSM303DLHC_ACCEL_ADDR 0x19
#define LSM303DLHC_MAG_ADDR   0x1E

/**
 * @brief LSM303DLHC Accel Full Scale
 */
typedef enum {
    LSM303DLHC_ACCEL_FS_2G  = 0,
    LSM303DLHC_ACCEL_FS_4G  = 1,
    LSM303DLHC_ACCEL_FS_8G  = 2,
    LSM303DLHC_ACCEL_FS_16G = 3,
} lsm303dlhc_accel_fs_t;

/**
 * @brief LSM303DLHC Accel Data Rate (ODR)
 * Only a subset for simplicity, see datasheet for all
 */
typedef enum {
    LSM303DLHC_ACCEL_ODR_OFF    = 0,
    LSM303DLHC_ACCEL_ODR_1HZ    = 1,
    LSM303DLHC_ACCEL_ODR_10HZ   = 2,
    LSM303DLHC_ACCEL_ODR_25HZ   = 3,
    LSM303DLHC_ACCEL_ODR_50HZ   = 4,
    LSM303DLHC_ACCEL_ODR_100HZ  = 5,
    LSM303DLHC_ACCEL_ODR_200HZ  = 6,
    LSM303DLHC_ACCEL_ODR_400HZ  = 7,
    LSM303DLHC_ACCEL_ODR_1344HZ = 9, 
} lsm303dlhc_accel_odr_t;

/**
 * @brief LSM303DLHC Mag Data Rate
 */
typedef enum {
    LSM303DLHC_MAG_ODR_0_75HZ = 0,
    LSM303DLHC_MAG_ODR_1_5HZ  = 1,
    LSM303DLHC_MAG_ODR_3_0HZ  = 2,
    LSM303DLHC_MAG_ODR_7_5HZ  = 3,
    LSM303DLHC_MAG_ODR_15HZ   = 4,
    LSM303DLHC_MAG_ODR_30HZ   = 5,
    LSM303DLHC_MAG_ODR_75HZ   = 6,
    LSM303DLHC_MAG_ODR_220HZ  = 7,
} lsm303dlhc_mag_odr_t;

/**
 * @brief LSM303DLHC Mag Gain
 */
typedef enum {
    LSM303DLHC_MAG_GAIN_1_3 = 1, // +/- 1.3 Gauss
    LSM303DLHC_MAG_GAIN_1_9 = 2,
    LSM303DLHC_MAG_GAIN_2_5 = 3,
    LSM303DLHC_MAG_GAIN_4_0 = 4,
    LSM303DLHC_MAG_GAIN_4_7 = 5,
    LSM303DLHC_MAG_GAIN_5_6 = 6,
    LSM303DLHC_MAG_GAIN_8_1 = 7,
} lsm303dlhc_mag_gain_t;

/**
 * @brief Configuration for INT1 pin routing (CTRL_REG3_A)
 */
typedef struct {
    bool i1_click;    /*!< CLICK interrupt on INT1 */
    bool i1_aoi1;     /*!< AOI1 interrupt on INT1 */
    bool i1_aoi2;     /*!< AOI2 interrupt on INT1 */
    bool i1_drdy1;    /*!< DRDY1 interrupt on INT1 */
    bool i1_drdy2;    /*!< DRDY2 interrupt on INT1 */
    bool i1_wtm;      /*!< FIFO watermark interrupt on INT1 */
    bool i1_overrun;  /*!< FIFO overrun interrupt on INT1 */
} lsm303dlhc_int1_pin_conf_t;

/**
 * @brief Configuration for INT2 pin routing (CTRL_REG6_A)
 */
typedef struct {
    bool i2_click;    /*!< CLICK interrupt on INT2 */
    bool i2_int1;     /*!< Interrupt 1 function on INT2 */
    bool i2_int2;     /*!< Interrupt 2 function on INT2 */
    bool boot_i1;     /*!< Boot on INT1 */
    bool p2_act;      /*!< Active function status on INT2 */
    bool h_lactive;   /*!< Interrupt active high (0) or low (1) */
} lsm303dlhc_int2_pin_conf_t;

/**
 * @brief Interrupt Generator Configuration (INT1_CFG_A / INT2_CFG_A)
 */
typedef struct {
    bool aoi;         /*!< And/Or combination of interrupt events */
    bool six_d;       /*!< 6-direction detection function enabled */
    bool zhie;        /*!< Enable interrupt generation on Z high event */
    bool zlie;        /*!< Enable interrupt generation on Z low event */
    bool yhie;        /*!< Enable interrupt generation on Y high event */
    bool ylie;        /*!< Enable interrupt generation on Y low event */
    bool xhie;        /*!< Enable interrupt generation on X high event */
    bool xlie;        /*!< Enable interrupt generation on X low event */
    uint8_t threshold;/*!< Interrupt threshold (7-bit) */
    uint8_t duration; /*!< Duration value (7-bit) */
} lsm303dlhc_int_event_conf_t;

typedef struct {
    i2c_port_t i2c_port;
    lsm303dlhc_accel_fs_t accel_fs;
    lsm303dlhc_accel_odr_t accel_odr;
    lsm303dlhc_mag_odr_t mag_odr;
    lsm303dlhc_mag_gain_t mag_gain;
} lsm303dlhc_config_t;

typedef struct lsm303dlhc_dev_t *lsm303dlhc_handle_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} lsm303dlhc_raw_data_t;

/**
 * @brief Initialize the LSM303DLHC sensor
 * 
 * @param config Configuration struct
 * @param ret_handle Pointer to return the handle
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_init(const lsm303dlhc_config_t *config, lsm303dlhc_handle_t *ret_handle);

/**
 * @brief Deinitialize and free resources
 * 
 * @param handle Device handle
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_deinit(lsm303dlhc_handle_t handle);

/**
 * @brief Read accelerometer raw data
 * 
 * @param handle Device handle
 * @param data Pointer to store X, Y, Z raw values
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_read_accel(lsm303dlhc_handle_t handle, lsm303dlhc_raw_data_t *data);

/**
 * @brief Read magnetometer raw data
 * 
 * @param handle Device handle
 * @param data Pointer to store X, Y, Z raw values
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_read_mag(lsm303dlhc_handle_t handle, lsm303dlhc_raw_data_t *data);

/**
 * @brief Configure INT1 pin routing
 * 
 * @param handle Device handle
 * @param conf Pointer to configuration struct
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_config_int1_pin(lsm303dlhc_handle_t handle, const lsm303dlhc_int1_pin_conf_t *conf);

/**
 * @brief Configure INT2 pin routing
 * 
 * @param handle Device handle
 * @param conf Pointer to configuration struct
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_config_int2_pin(lsm303dlhc_handle_t handle, const lsm303dlhc_int2_pin_conf_t *conf);

/**
 * @brief Configure Interrupt 1 Event Generator
 * 
 * @param handle Device handle
 * @param conf Pointer to configuration struct
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_config_int1_event(lsm303dlhc_handle_t handle, const lsm303dlhc_int_event_conf_t *conf);

/**
 * @brief Configure Interrupt 2 Event Generator
 * 
 * @param handle Device handle
 * @param conf Pointer to configuration struct
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_config_int2_event(lsm303dlhc_handle_t handle, const lsm303dlhc_int_event_conf_t *conf);

/**
 * @brief Get Interrupt 1 Source (clears the interrupt)
 * 
 * @param handle Device handle
 * @param src Pointer to byte to store source register value
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_get_int1_src(lsm303dlhc_handle_t handle, uint8_t *src);

/**
 * @brief Get Interrupt 2 Source (clears the interrupt)
 * 
 * @param handle Device handle
 * @param src Pointer to byte to store source register value
 * @return esp_err_t 
 */
esp_err_t lsm303dlhc_get_int2_src(lsm303dlhc_handle_t handle, uint8_t *src);

#ifdef __cplusplus
}
#endif
