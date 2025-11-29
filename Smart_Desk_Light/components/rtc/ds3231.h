#ifndef DS3231_H
#define DS3231_H

#include "esp_err.h"
#include "driver/i2c_master.h"

// Địa chỉ DS3231
#define DS3231_ADDR 0x68

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// Cần lưu bus handle (I2C mới bắt buộc)
extern i2c_master_bus_handle_t ds3231_bus;

esp_err_t ds3231_init(i2c_master_bus_handle_t bus);
esp_err_t ds3231_get_time(rtc_time_t *t);
esp_err_t ds3231_set_time(rtc_time_t t);

#endif
