#include "ds3231.h"
#include "esp_log.h"

static const char *TAG = "DS3231";

i2c_master_bus_handle_t ds3231_bus = NULL;
i2c_master_dev_handle_t ds3231_dev = NULL;

// Hàm đổi Binary ↔ BCD
static uint8_t bcd2dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t dec2bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

esp_err_t ds3231_init(i2c_master_bus_handle_t bus)
{
    ds3231_bus = bus;

    i2c_device_config_t dev_cfg = {
        .device_address = DS3231_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(ds3231_bus, &dev_cfg, &ds3231_dev));

    ESP_LOGI(TAG, "DS3231 initialized");
    return ESP_OK;
}

esp_err_t ds3231_get_time(rtc_time_t *t)
{
    uint8_t reg = 0x00;     // Register bắt đầu đọc
    uint8_t data[7];

    // Gửi địa chỉ register
    ESP_ERROR_CHECK(i2c_master_transmit(ds3231_dev, &reg, 1, -1));

    // Đọc 7 byte thời gian
    ESP_ERROR_CHECK(i2c_master_receive(ds3231_dev, data, 7, -1));

    t->sec   = bcd2dec(data[0] & 0x7F);
    t->min   = bcd2dec(data[1]);
    t->hour  = bcd2dec(data[2] & 0x3F);
    t->day   = bcd2dec(data[4]);
    t->month = bcd2dec(data[5] & 0x1F);
    t->year  = 2000 + bcd2dec(data[6]);

    return ESP_OK;
}

esp_err_t ds3231_set_time(rtc_time_t t)
{
    uint8_t buf[8];
    buf[0] = 0x00;  // Register bắt đầu
    buf[1] = dec2bcd(t.sec);
    buf[2] = dec2bcd(t.min);
    buf[3] = dec2bcd(t.hour);
    buf[4] = dec2bcd(1); // Day of week (không quan trọng)
    buf[5] = dec2bcd(t.day);
    buf[6] = dec2bcd(t.month);
    buf[7] = dec2bcd(t.year - 2000);

    return i2c_master_transmit(ds3231_dev, buf, 8, -1);
}
