#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

// ===== ĐỊNH NGHĨA TAG =====
static const char *TAG = "RTC_OLED_SPI";

// ===== CẤU HÌNH I2C CHO DS3231 =====
#define I2C_MASTER_SCL_IO       GPIO_NUM_7
#define I2C_MASTER_SDA_IO       GPIO_NUM_6
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000
#define DS3231_ADDR             0x68

// ===== CẤU HÌNH SPI CHO OLED =====
#define OLED_DC_GPIO            GPIO_NUM_4    // D/C pin
#define OLED_CS_GPIO            GPIO_NUM_5    // CS pin
#define OLED_MOSI_GPIO          GPIO_NUM_2    // D1 (MOSI)
#define OLED_SCLK_GPIO          GPIO_NUM_3    // D0 (SCK/CLK)
#define OLED_RST_GPIO           GPIO_NUM_10   // RST (reset pin)

#define OLED_WIDTH              128
#define OLED_HEIGHT             64

// ===== CẤU TRÚC THỜI GIAN =====
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// ===== SPI HANDLE =====
static spi_device_handle_t spi_device;

// ===== FONT 8x8 =====
static const uint8_t font_8x8[][8] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00, 0x00}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, 0x00}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x00}, // 9
    {0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00}, // :
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, 0x00, 0x00}, // /
};

// ===== CHUYỂN ĐỔI BCD =====
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) * 16) + (dec % 10);
}

// ===== KHỞI TẠO I2C =====
esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// ===== GHI LỆNH OLED SPI =====
void oled_write_cmd(uint8_t cmd) {
    gpio_set_level(OLED_DC_GPIO, 0);  // Command mode
    
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_device, &t);
}

// ===== GHI DỮ LIỆU OLED SPI =====
void oled_write_data(uint8_t *data, size_t len) {
    gpio_set_level(OLED_DC_GPIO, 1);  // Data mode
    
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_device, &t);
}

// ===== RESET OLED =====
void oled_reset(void) {
    gpio_set_level(OLED_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(OLED_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "OLED reset complete");
}

// ===== KHỞI TẠO SPI =====
void spi_master_init(void) {
    // Cấu hình bus SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = OLED_MOSI_GPIO,
        .miso_io_num = -1,  // Không dùng MISO
        .sclk_io_num = OLED_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    // Cấu hình device OLED - Thử mode 0 hoặc mode 3
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,  // Giảm xuống 8 MHz để ổn định hơn
        .mode = 0,  // SPI mode 0 (nếu không work thử mode 3)
        .spics_io_num = OLED_CS_GPIO,
        .queue_size = 7,
        .flags = 0,
    };
    
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device));
    
    // Cấu hình DC pin
    gpio_config_t io_conf_dc = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_DC_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_dc);
    
    // Cấu hình RST pin
    gpio_config_t io_conf_rst = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_RST_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_rst);
    
    ESP_LOGI(TAG, "SPI initialized");
}

// ===== KHỞI TẠO OLED =====
void oled_init(void) {
    // Reset OLED
    oled_reset();
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "Starting OLED initialization sequence...");
    
    oled_write_cmd(0xAE);  // Display off
    
    oled_write_cmd(0xD5);  // Set clock divide
    oled_write_cmd(0x80);
    
    oled_write_cmd(0xA8);  // Set multiplex
    oled_write_cmd(0x3F);  // 64 rows
    
    oled_write_cmd(0xD3);  // Set display offset
    oled_write_cmd(0x00);
    
    oled_write_cmd(0x40);  // Set start line address
    
    oled_write_cmd(0x8D);  // Charge pump
    oled_write_cmd(0x14);  // Enable charge pump
    
    oled_write_cmd(0x20);  // Memory addressing mode
    oled_write_cmd(0x00);  // Horizontal addressing mode
    
    oled_write_cmd(0xA1);  // Segment remap (A0/A1)
    oled_write_cmd(0xC8);  // COM scan direction (C0/C8)
    
    oled_write_cmd(0xDA);  // COM pins hardware configuration
    oled_write_cmd(0x12);
    
    oled_write_cmd(0x81);  // Contrast control
    oled_write_cmd(0xCF);
    
    oled_write_cmd(0xD9);  // Pre-charge period
    oled_write_cmd(0xF1);
    
    oled_write_cmd(0xDB);  // VCOMH deselect level
    oled_write_cmd(0x40);
    
    oled_write_cmd(0xA4);  // Display all on resume
    oled_write_cmd(0xA6);  // Normal display (not inverted)
    
    oled_write_cmd(0x2E);  // Deactivate scroll
    
    oled_write_cmd(0xAF);  // Display on
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "OLED initialized successfully");
}

// ===== XÓA MÀN HÌNH =====
void oled_clear(void) {
    uint8_t clear[128] = {0};
    
    for (int page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page);  // Set page address
        oled_write_cmd(0x00);          // Set lower column address
        oled_write_cmd(0x10);          // Set higher column address
        oled_write_data(clear, 128);
    }
}

// ===== VẼ KÝ TỰ LỚN =====
void oled_draw_large_char(uint8_t page, uint8_t col, uint8_t c) {
    if (c == ':') c = 10;
    else if (c == '/') c = 11;
    else if (c >= '0' && c <= '9') c = c - '0';
    else return;
    
    for (int p = 0; p < 2; p++) {
        oled_write_cmd(0xB0 + page + p);
        oled_write_cmd(0x00 + (col & 0x0F));
        oled_write_cmd(0x10 + (col >> 4));
        
        for (int i = 0; i < 8; i++) {
            uint8_t data = font_8x8[c][i];
            uint8_t byte = 0;
            
            if (p == 0) {
                for (int b = 0; b < 4; b++) {
                    if (data & (1 << b)) byte |= (3 << (b * 2));
                }
            } else {
                for (int b = 0; b < 4; b++) {
                    if (data & (1 << (b + 4))) byte |= (3 << (b * 2));
                }
            }
            oled_write_data(&byte, 1);
            oled_write_data(&byte, 1);
        }
    }
}

// ===== HIỂN THỊ THỜI GIAN =====
void oled_display_time(rtc_time_t *time) {
    // Hiển thị giờ lớn (page 2-3)
    oled_draw_large_char(2, 10, time->hours / 10 + '0');
    oled_draw_large_char(2, 26, time->hours % 10 + '0');
    oled_draw_large_char(2, 46, ':');
    oled_draw_large_char(2, 58, time->minutes / 10 + '0');
    oled_draw_large_char(2, 74, time->minutes % 10 + '0');
    oled_draw_large_char(2, 94, ':');
    oled_draw_large_char(2, 106, time->seconds / 10 + '0');
    oled_draw_large_char(2, 122, time->seconds % 10 + '0');  // ĐÃ SỬA: Thêm chữ số cuối
    
    // Hiển thị ngày (page 5-6)
    oled_draw_large_char(5, 10, time->date / 10 + '0');
    oled_draw_large_char(5, 26, time->date % 10 + '0');
    oled_draw_large_char(5, 42, '/');
    oled_draw_large_char(5, 54, time->month / 10 + '0');
    oled_draw_large_char(5, 70, time->month % 10 + '0');
}

// ===== ĐỌC THỜI GIAN DS3231 =====
esp_err_t ds3231_get_time(rtc_time_t *time) {
    uint8_t reg = 0x00;
    uint8_t data[7];
    
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, DS3231_ADDR,
                                                &reg, 1, data, 7, 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from DS3231: %s", esp_err_to_name(err));
        return err;
    }
    
    time->seconds = bcd_to_dec(data[0] & 0x7F);
    time->minutes = bcd_to_dec(data[1] & 0x7F);
    time->hours   = bcd_to_dec(data[2] & 0x3F);
    time->day     = bcd_to_dec(data[3] & 0x07);
    time->date    = bcd_to_dec(data[4] & 0x3F);
    time->month   = bcd_to_dec(data[5] & 0x1F);
    time->year    = bcd_to_dec(data[6]) + 2000;
    
    return ESP_OK;
}

// ===== ĐẶT THỜI GIAN =====
esp_err_t ds3231_set_time(rtc_time_t *time) {
    uint8_t data[8];
    data[0] = 0x00;
    data[1] = dec_to_bcd(time->seconds);
    data[2] = dec_to_bcd(time->minutes);
    data[3] = dec_to_bcd(time->hours);
    data[4] = dec_to_bcd(time->day);
    data[5] = dec_to_bcd(time->date);
    data[6] = dec_to_bcd(time->month);
    data[7] = dec_to_bcd(time->year - 2000);
    
    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, DS3231_ADDR,
                                               data, 8, 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set time: %s", esp_err_to_name(err));
    }
    return err;
}

// ===== TASK HIỂN THỊ =====
void display_task(void *pvParameter) {
    rtc_time_t time;
    
    while (1) {
        if (ds3231_get_time(&time) == ESP_OK) {
            oled_clear();
            oled_display_time(&time);
            
            ESP_LOGI(TAG, "%02d:%02d:%02d - %02d/%02d/%04d",
                     time.hours, time.minutes, time.seconds,
                     time.date, time.month, time.year);
        } else {
            ESP_LOGE(TAG, "Failed to read time from DS3231");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ===== TEST OLED (Vẽ pattern để kiểm tra) =====
void oled_test_pattern(void) {
    ESP_LOGI(TAG, "Drawing test pattern...");
    
    // Vẽ các dòng ngang
    uint8_t line[128];
    memset(line, 0xFF, 128);
    
    for (int page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        
        if (page % 2 == 0) {
            oled_write_data(line, 128);
        } else {
            uint8_t empty[128] = {0};
            oled_write_data(empty, 128);
        }
    }
    
    ESP_LOGI(TAG, "Test pattern complete");
}

// ===== MAIN =====
void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32-C3 RTC DS3231 + OLED SPI");
    ESP_LOGI(TAG, "========================================");
    
    // Khởi tạo I2C cho DS3231
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized");
    
    // Khởi tạo SPI cho OLED
    spi_master_init();
    oled_init();
    
    // Test OLED với pattern đơn giản
    ESP_LOGI(TAG, "Testing OLED...");
    oled_test_pattern();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    oled_clear();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // ===== ĐẶT THỜI GIAN BAN ĐẦU =====
    rtc_time_t init_time = {
        .seconds = 0,
        .minutes = 38,
        .hours = 12,
        .day = 7,
        .date = 29,
        .month = 11,
        .year = 2025
    };
    
    if (ds3231_set_time(&init_time) == ESP_OK) {
        ESP_LOGI(TAG, "Time set successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to set time!");
    }
    
    ESP_LOGI(TAG, "Starting display...");
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
}