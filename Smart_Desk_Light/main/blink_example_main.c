// #include <stdio.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/i2c.h"
// #include "driver/gpio.h"
// #include "driver/spi_master.h"
// #include "esp_log.h"

// // ===== ĐỊNH NGHĨA TAG =====
// static const char *TAG = "RTC_OLED_SPI";

// // ===== CẤU HÌNH I2C CHO DS3231 =====
// #define I2C_MASTER_SCL_IO       GPIO_NUM_7
// #define I2C_MASTER_SDA_IO       GPIO_NUM_6
// #define I2C_MASTER_NUM          I2C_NUM_0
// #define I2C_MASTER_FREQ_HZ      100000
// #define DS3231_ADDR             0x68

// // ===== CẤU HÌNH SPI CHO OLED =====
// #define OLED_DC_GPIO            GPIO_NUM_4
// #define OLED_CS_GPIO            GPIO_NUM_5
// #define OLED_MOSI_GPIO          GPIO_NUM_2
// #define OLED_SCLK_GPIO          GPIO_NUM_3
// #define OLED_RST_GPIO           GPIO_NUM_10

// // ===== CẤU HÌNH NÚT NHẤN CHO POMODORO =====
// #define BTN_START_STOP_GPIO     GPIO_NUM_18    // Nút Start/Stop
// #define BTN_RESET_GPIO          GPIO_NUM_19    // Nút Reset

// #define OLED_WIDTH              128
// #define OLED_HEIGHT             64

// // ===== POMODORO SETTINGS =====
// #define POMODORO_WORK_DURATION      (25 * 60)     // 25 phút
// #define POMODORO_BREAK_DURATION     (5 * 60)      // 5 phút
// #define POMODORO_LONG_BREAK_DURATION (15 * 60)    // 15 phút

// // ===== CẤU TRÚC THỜI GIAN =====
// typedef struct {
//     uint8_t seconds;
//     uint8_t minutes;
//     uint8_t hours;
//     uint8_t day;
//     uint8_t date;
//     uint8_t month;
//     uint16_t year;
// } rtc_time_t;

// // ===== CẤU TRÚC POMODORO =====
// typedef enum {
//     POMODORO_IDLE,
//     POMODORO_WORK,
//     POMODORO_BREAK,
//     POMODORO_LONG_BREAK
// } pomodoro_state_t;

// typedef struct {
//     pomodoro_state_t state;
//     bool is_running;
//     uint16_t time_left;         // Giây còn lại
//     uint8_t pomodoro_count;     // Số pomodoro đã hoàn thành
// } pomodoro_t;

// // ===== BIẾN TOÀN CỤC =====
// static spi_device_handle_t spi_device;
// static pomodoro_t pomodoro = {
//     .state = POMODORO_IDLE,
//     .is_running = false,
//     .time_left = POMODORO_WORK_DURATION,
//     .pomodoro_count = 0
// };

// // ===== FONT 8x8 =====
// static const uint8_t font_8x8[][8] = {
//     {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00, 0x00}, // 0
//     {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, // 1
//     {0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, 0x00}, // 2
//     {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00, 0x00}, // 3
//     {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00, 0x00}, // 4
//     {0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, 0x00}, // 5
//     {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00, 0x00}, // 6
//     {0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, 0x00}, // 7
//     {0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00}, // 8
//     {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x00}, // 9
//     {0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00}, // :
//     {0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00}  // -
// };

// // XÓA font_5x7 vì không cần thiết

// // ===== CHUYỂN ĐỔI BCD =====
// uint8_t bcd_to_dec(uint8_t bcd) {
//     return ((bcd / 16) * 10) + (bcd % 16);
// }

// uint8_t dec_to_bcd(uint8_t dec) {
//     return ((dec / 10) * 16) + (dec % 10);
// }

// // ===== KHỞI TẠO I2C =====
// esp_err_t i2c_master_init(void) {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = I2C_MASTER_FREQ_HZ,
//     };
    
//     esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
//     if (err != ESP_OK) return err;
    
//     return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
// }
// // ===== ĐẶT THỜI GIAN =====
// esp_err_t ds3231_set_time(rtc_time_t *time) {
//     uint8_t data[8];
//     data[0] = 0x00;
//     data[1] = dec_to_bcd(time->seconds);
//     data[2] = dec_to_bcd(time->minutes);
//     data[3] = dec_to_bcd(time->hours);
//     data[4] = dec_to_bcd(time->day);
//     data[5] = dec_to_bcd(time->date);
//     data[6] = dec_to_bcd(time->month);
//     data[7] = dec_to_bcd(time->year - 2000);
    
//     esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, DS3231_ADDR,
//                                                data, 8, 1000 / portTICK_PERIOD_MS);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to set time: %s", esp_err_to_name(err));
//     }
//     return err;
// }

// // ===== KHỞI TẠO NÚT NHẤN =====
// void button_init(void) {
//     gpio_config_t io_conf = {
//         .intr_type = GPIO_INTR_DISABLE,
//         .mode = GPIO_MODE_INPUT,
//         .pin_bit_mask = (1ULL << BTN_START_STOP_GPIO) | (1ULL << BTN_RESET_GPIO),
//         .pull_down_en = 0,
//         .pull_up_en = 1,
//     };
//     gpio_config(&io_conf);
//     ESP_LOGI(TAG, "Buttons initialized");
// }

// // ===== GHI LỆNH OLED SPI =====
// void oled_write_cmd(uint8_t cmd) {
//     gpio_set_level(OLED_DC_GPIO, 0);
//     spi_transaction_t t = {
//         .length = 8,
//         .tx_buffer = &cmd,
//         .flags = 0,
//     };
//     spi_device_polling_transmit(spi_device, &t);
// }

// // ===== GHI DỮ LIỆU OLED SPI =====
// void oled_write_data(uint8_t *data, size_t len) {
//     gpio_set_level(OLED_DC_GPIO, 1);
//     spi_transaction_t t = {
//         .length = len * 8,
//         .tx_buffer = data,
//         .flags = 0,
//     };
//     spi_device_polling_transmit(spi_device, &t);
// }

// // ===== RESET OLED =====
// void oled_reset(void) {
//     gpio_set_level(OLED_RST_GPIO, 0);
//     vTaskDelay(pdMS_TO_TICKS(10));
//     gpio_set_level(OLED_RST_GPIO, 1);
//     vTaskDelay(pdMS_TO_TICKS(10));
//     ESP_LOGI(TAG, "OLED reset complete");
// }

// // ===== KHỞI TẠO SPI =====
// void spi_master_init(void) {
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = OLED_MOSI_GPIO,
//         .miso_io_num = -1,
//         .sclk_io_num = OLED_SCLK_GPIO,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//         .max_transfer_sz = 4096,
//     };
    
//     ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
//     spi_device_interface_config_t devcfg = {
//         .clock_speed_hz = 8 * 1000 * 1000,
//         .mode = 0,
//         .spics_io_num = OLED_CS_GPIO,
//         .queue_size = 7,
//         .flags = 0,
//     };
    
//     ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device));
    
//     gpio_config_t io_conf_dc = {
//         .intr_type = GPIO_INTR_DISABLE,
//         .mode = GPIO_MODE_OUTPUT,
//         .pin_bit_mask = (1ULL << OLED_DC_GPIO),
//         .pull_down_en = 0,
//         .pull_up_en = 0,
//     };
//     gpio_config(&io_conf_dc);
    
//     gpio_config_t io_conf_rst = {
//         .intr_type = GPIO_INTR_DISABLE,
//         .mode = GPIO_MODE_OUTPUT,
//         .pin_bit_mask = (1ULL << OLED_RST_GPIO),
//         .pull_down_en = 0,
//         .pull_up_en = 0,
//     };
//     gpio_config(&io_conf_rst);
    
//     ESP_LOGI(TAG, "SPI initialized");
// }

// // ===== KHỞI TẠO OLED =====
// void oled_init(void) {
//     oled_reset();
//     vTaskDelay(pdMS_TO_TICKS(100));
    
//     ESP_LOGI(TAG, "Starting OLED initialization sequence...");
    
//     oled_write_cmd(0xAE);
//     oled_write_cmd(0xD5); oled_write_cmd(0x80);
//     oled_write_cmd(0xA8); oled_write_cmd(0x3F);
//     oled_write_cmd(0xD3); oled_write_cmd(0x00);
//     oled_write_cmd(0x40);
//     oled_write_cmd(0x8D); oled_write_cmd(0x14);
//     oled_write_cmd(0x20); oled_write_cmd(0x00);
//     oled_write_cmd(0xA1);
//     oled_write_cmd(0xC8);
//     oled_write_cmd(0xDA); oled_write_cmd(0x12);
//     oled_write_cmd(0x81); oled_write_cmd(0xCF);
//     oled_write_cmd(0xD9); oled_write_cmd(0xF1);
//     oled_write_cmd(0xDB); oled_write_cmd(0x40);
//     oled_write_cmd(0xA4);
//     oled_write_cmd(0xA6);
//     oled_write_cmd(0x2E);
//     oled_write_cmd(0xAF);
    
//     vTaskDelay(pdMS_TO_TICKS(100));
//     ESP_LOGI(TAG, "OLED initialized successfully");
// }

// // ===== XÓA MÀN HÌNH =====
// void oled_clear(void) {
//     uint8_t clear[128] = {0};
//     for (int page = 0; page < 8; page++) {
//         oled_write_cmd(0xB0 + page);
//         oled_write_cmd(0x00);
//         oled_write_cmd(0x10);
//         oled_write_data(clear, 128);
//     }
// }

// // ===== VẼ KÝ TỰ LỚN =====
// void oled_draw_large_char(uint8_t page, uint8_t col, uint8_t c) {
//     if (c == ':') c = 10;
//     else if (c == '/') c = 11;
//     else if (c >= '0' && c <= '9') c = c - '0';
//     else return;
    
//     for (int p = 0; p < 2; p++) {
//         oled_write_cmd(0xB0 + page + p);
//         oled_write_cmd(0x00 + (col & 0x0F));
//         oled_write_cmd(0x10 + (col >> 4));
        
//         for (int i = 0; i < 8; i++) {
//             uint8_t data = font_8x8[c][i];
//             uint8_t byte = 0;
            
//             if (p == 0) {
//                 for (int b = 0; b < 4; b++) {
//                     if (data & (1 << b)) byte |= (3 << (b * 2));
//                 }
//             } else {
//                 for (int b = 0; b < 4; b++) {
//                     if (data & (1 << (b + 4))) byte |= (3 << (b * 2));
//                 }
//             }
//             oled_write_data(&byte, 1);
//             oled_write_data(&byte, 1);
//         }
//     }
// }

// // ===== VẼ KÝ TỰ NHỎ =====
// void oled_draw_small_char(uint8_t page, uint8_t col, uint8_t c) {
//     if (c >= '0' && c <= '9') c = c - '0';
//     else if (c == ':') c = 10;
//     else return;
    
//     oled_write_cmd(0xB0 + page);
//     oled_write_cmd(0x00 + (col & 0x0F));
//     oled_write_cmd(0x10 + (col >> 4));
    
//     for (int i = 0; i < 8; i++) {
//         uint8_t byte = font_8x8[c][i];
//         oled_write_data(&byte, 1);
//     }
// }

// // ===== HIỂN THỊ POMODORO TIMER =====
// void oled_display_pomodoro(void) {
//     // Hiển thị ở page 5-6 (dưới dòng giờ)
//     uint8_t minutes = pomodoro.time_left / 60;
//     uint8_t seconds = pomodoro.time_left % 60;
    
//     // Vẽ timer: MM:SS (dùng font nhỏ 8x8 thông thường)
//     int offset = 30;
    
//     // Vẽ phút
//     oled_draw_small_char(5, offset, minutes / 10 + '0');
//     oled_draw_small_char(5, offset + 8, minutes % 10 + '0');
    
//     // Vẽ dấu :
//     oled_draw_small_char(5, offset + 16, ':');
    
//     // Vẽ giây
//     oled_draw_small_char(5, offset + 24, seconds / 10 + '0');
//     oled_draw_small_char(5, offset + 32, seconds % 10 + '0');
    
//     // Hiển thị trạng thái W (Work) hoặc B (Break)
//     oled_write_cmd(0xB0 + 5);
//     oled_write_cmd(0x00 + ((offset + 45) & 0x0F));
//     oled_write_cmd(0x10 + ((offset + 45) >> 4));
    
//     if (pomodoro.state == POMODORO_WORK) {
//         // Vẽ chữ W
//         uint8_t w[] = {0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00};
//         oled_write_data(w, 6);
//     } else if (pomodoro.state == POMODORO_BREAK || pomodoro.state == POMODORO_LONG_BREAK) {
//         // Vẽ chữ B
//         uint8_t b[] = {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00};
//         oled_write_data(b, 6);
//     }
    
//     // Hiển thị số Pomodoro đã hoàn thành (chỉ hiển thị nếu < 10)
//     if (pomodoro.pomodoro_count < 10) {
//         oled_draw_small_char(5, offset + 55, pomodoro.pomodoro_count + '0');
//     }
// }

// // ===== HIỂN THỊ CHỈ GIỜ =====
// void oled_display_time_only(rtc_time_t *time) {
//     int add = 15;
//     oled_draw_large_char(2, add, time->hours / 10 + '0');
//     oled_draw_large_char(2, 16 + add, time->hours % 10 + '0');
//     oled_draw_large_char(2, 28 + add, ':');
//     oled_draw_large_char(2, 40 + add, time->minutes / 10 + '0');
//     oled_draw_large_char(2, 52 + add, time->minutes % 10 + '0');
//     oled_draw_large_char(2, 64 + add, ':');
//     oled_draw_large_char(2, 76 + add, time->seconds / 10 + '0');
//     oled_draw_large_char(2, 88 + add, time->seconds % 10 + '0');
// }

// // ===== HIỂN THỊ CHỈ NGÀY =====
// void oled_display_date_only(rtc_time_t *time) {
//     int add = 25;
//     oled_draw_large_char(2, add, time->date / 10 + '0');
//     oled_draw_large_char(2, 16 + add, time->date % 10 + '0');
//     oled_draw_large_char(2, 32 + add, '/');
//     oled_draw_large_char(2, 48 + add, time->month / 10 + '0');
//     oled_draw_large_char(2, 64 + add, time->month % 10 + '0');
    
//     int offset = 30;
//     uint16_t year = time->year;
//     oled_draw_large_char(5, offset, (year / 1000) + '0');
//     oled_draw_large_char(5, 16 + offset, ((year / 100) % 10) + '0');
//     oled_draw_large_char(5, 32 + offset, ((year / 10) % 10) + '0');
//     oled_draw_large_char(5, 48 + offset, (year % 10) + '0');
// }

// // ===== ĐỌC THỜI GIAN DS3231 =====
// esp_err_t ds3231_get_time(rtc_time_t *time) {
//     uint8_t reg = 0x00;
//     uint8_t data[7];
    
//     esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, DS3231_ADDR,
//                                                 &reg, 1, data, 7, 1000 / portTICK_PERIOD_MS);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to read from DS3231: %s", esp_err_to_name(err));
//         return err;
//     }
    
//     time->seconds = bcd_to_dec(data[0] & 0x7F);
//     time->minutes = bcd_to_dec(data[1] & 0x7F);
//     time->hours   = bcd_to_dec(data[2] & 0x3F);
//     time->day     = bcd_to_dec(data[3] & 0x07);
//     time->date    = bcd_to_dec(data[4] & 0x3F);
//     time->month   = bcd_to_dec(data[5] & 0x1F);
//     time->year    = bcd_to_dec(data[6]) + 2000;
    
//     return ESP_OK;
// }

// // ===== XỬ LÝ POMODORO =====
// void pomodoro_start_stop(void) {
//     pomodoro.is_running = !pomodoro.is_running;
//     ESP_LOGI(TAG, "Pomodoro %s", pomodoro.is_running ? "Started" : "Paused");
// }

// void pomodoro_reset(void) {
//     pomodoro.is_running = false;
//     pomodoro.state = POMODORO_WORK;
//     pomodoro.time_left = POMODORO_WORK_DURATION;
//     ESP_LOGI(TAG, "Pomodoro Reset");
// }

// void pomodoro_tick(void) {
//     if (!pomodoro.is_running) return;
    
//     if (pomodoro.time_left > 0) {
//         pomodoro.time_left--;
//     } else {
//         // Hết thời gian
//         if (pomodoro.state == POMODORO_WORK) {
//             pomodoro.pomodoro_count++;
            
//             if (pomodoro.pomodoro_count % 4 == 0) {
//                 pomodoro.state = POMODORO_LONG_BREAK;
//                 pomodoro.time_left = POMODORO_LONG_BREAK_DURATION;
//                 ESP_LOGI(TAG, "Long Break!");
//             } else {
//                 pomodoro.state = POMODORO_BREAK;
//                 pomodoro.time_left = POMODORO_BREAK_DURATION;
//                 ESP_LOGI(TAG, "Short Break!");
//             }
//         } else {
//             pomodoro.state = POMODORO_WORK;
//             pomodoro.time_left = POMODORO_WORK_DURATION;
//             ESP_LOGI(TAG, "Work Time!");
//         }
//         pomodoro.is_running = false; // Tự động dừng khi hết thời gian
//     }
// }

// // ===== TASK NÚT NHẤN =====
// void button_task(void *pvParameter) {
//     bool btn_start_last = true;
//     bool btn_reset_last = true;
    
//     while (1) {
//         bool btn_start = gpio_get_level(BTN_START_STOP_GPIO);
//         bool btn_reset = gpio_get_level(BTN_RESET_GPIO);
        
//         // Phát hiện nhấn nút (active LOW - nhấn = 0)
//         if (!btn_start && btn_start_last) {
//             vTaskDelay(pdMS_TO_TICKS(50)); // Debounce
//             if (!gpio_get_level(BTN_START_STOP_GPIO)) {
//                 pomodoro_start_stop();
//             }
//         }
        
//         if (!btn_reset && btn_reset_last) {
//             vTaskDelay(pdMS_TO_TICKS(50)); // Debounce
//             if (!gpio_get_level(BTN_RESET_GPIO)) {
//                 pomodoro_reset();
//             }
//         }
        
//         btn_start_last = btn_start;
//         btn_reset_last = btn_reset;
        
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// // ===== TASK HIỂN THỊ =====
// void display_task(void *pvParameter) {
//     rtc_time_t time;
//     bool show_time = true;
//     uint8_t counter = 0;
    
//     while (1) {
//         if (ds3231_get_time(&time) == ESP_OK) {
//             oled_clear();
            
//             if (show_time) {
//                 // Hiển thị thời gian
//                 oled_display_time_only(&time);
//                 // Hiển thị Pomodoro LUÔN khi đang ở chế độ hiển thị giờ
//                 oled_display_pomodoro();
                
//                 ESP_LOGI(TAG, "TIME: %02d:%02d:%02d | POMO: %02d:%02d %s",
//                          time.hours, time.minutes, time.seconds,
//                          pomodoro.time_left / 60, pomodoro.time_left % 60,
//                          pomodoro.is_running ? "RUN" : "PAUSE");
//             } else {
//                 // Chỉ hiển thị ngày, KHÔNG hiển thị Pomodoro
//                 oled_display_date_only(&time);
                
//                 ESP_LOGI(TAG, "DATE: %02d/%02d/%04d",
//                          time.date, time.month, time.year);
//             }
            
//             counter++;
//             if (counter >= 3) {
//                 counter = 0;
//                 show_time = !show_time;
//             }
//         } else {
//             ESP_LOGE(TAG, "Failed to read time from DS3231");
//         }
        
//         // Tick Pomodoro mỗi giây
//         pomodoro_tick();
        
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// // ===== MAIN =====
// void app_main(void) {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "  ESP32-C3 RTC + OLED + Pomodoro");
//     ESP_LOGI(TAG, "========================================");
    
//     ESP_ERROR_CHECK(i2c_master_init());
//     ESP_LOGI(TAG, "I2C initialized");
    
//     spi_master_init();
//     oled_init();
//     oled_clear();
    
//     button_init();
    
//     vTaskDelay(pdMS_TO_TICKS(500));
//     //     // ===== ĐẶT THỜI GIAN BAN ĐẦU =====
//     // rtc_time_t init_time = {
//     //     .seconds = 0,
//     //     .minutes = 48,
//     //     .hours = 9,
//     //     .day = 5,
//     //     .date = 12,
//     //     .month = 11,
//     //     .year = 2025
//     // };
    
//     // if (ds3231_set_time(&init_time) == ESP_OK) {
//     //     ESP_LOGI(TAG, "Time set successfully!");
//     // } else {
//     //     ESP_LOGE(TAG, "Failed to set time!");
//     // }
//     // Khởi tạo Pomodoro
//     pomodoro_reset();
    
//     ESP_LOGI(TAG, "Starting display...");
//     xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
//     xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
// }


-------------------------

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_netif.h"

// ===== ĐỊNH NGHĨA TAG =====
static const char *TAG = "RTC_OLED_WEB";

// ===== CẤU HÌNH WIFI =====
#define WIFI_SSID      "TTHL&TT-DHBK"
#define WIFI_PASS      "viettenbadualara"

// ===== CẤU HÌNH I2C CHO DS3231 =====
#define I2C_MASTER_SCL_IO       GPIO_NUM_7
#define I2C_MASTER_SDA_IO       GPIO_NUM_6
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000
#define DS3231_ADDR             0x68

// ===== CẤU HÌNH SPI CHO OLED =====
#define OLED_DC_GPIO            GPIO_NUM_4
#define OLED_CS_GPIO            GPIO_NUM_5
#define OLED_MOSI_GPIO          GPIO_NUM_2
#define OLED_SCLK_GPIO          GPIO_NUM_3
#define OLED_RST_GPIO           GPIO_NUM_10

// ===== CẤU HÌNH NÚT NHẤN CHO POMODORO =====
#define BTN_START_STOP_GPIO     GPIO_NUM_18
#define BTN_RESET_GPIO          GPIO_NUM_19

#define OLED_WIDTH              128
#define OLED_HEIGHT             64

// ===== POMODORO SETTINGS =====
#define POMODORO_WORK_DURATION      (25 * 60)
#define POMODORO_BREAK_DURATION     (5 * 60)
#define POMODORO_LONG_BREAK_DURATION (15 * 60)

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

// ===== CẤU TRÚC POMODORO =====
typedef enum {
    POMODORO_IDLE,
    POMODORO_WORK,
    POMODORO_BREAK,
    POMODORO_LONG_BREAK
} pomodoro_state_t;

typedef struct {
    pomodoro_state_t state;
    bool is_running;
    uint16_t time_left;
    uint8_t pomodoro_count;
} pomodoro_t;

// ===== BIẾN TOÀN CỤC =====
static spi_device_handle_t spi_device;
static pomodoro_t pomodoro = {
    .state = POMODORO_WORK,
    .is_running = false,
    .time_left = POMODORO_WORK_DURATION,
    .pomodoro_count = 0
};
static rtc_time_t current_time;
static httpd_handle_t server = NULL;

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
    {0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00}  // -
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

// ===== KHỞI TẠO NÚT NHẤN =====
void button_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN_START_STOP_GPIO) | (1ULL << BTN_RESET_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "Buttons initialized");
}

// ===== GHI LỆNH OLED SPI =====
void oled_write_cmd(uint8_t cmd) {
    gpio_set_level(OLED_DC_GPIO, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_device, &t);
}

// ===== GHI DỮ LIỆU OLED SPI =====
void oled_write_data(uint8_t *data, size_t len) {
    gpio_set_level(OLED_DC_GPIO, 1);
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
}

// ===== KHỞI TẠO SPI =====
void spi_master_init(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = OLED_MOSI_GPIO,
        .miso_io_num = -1,
        .sclk_io_num = OLED_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = OLED_CS_GPIO,
        .queue_size = 7,
        .flags = 0,
    };
    
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device));
    
    gpio_config_t io_conf_dc = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_DC_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_dc);
    
    gpio_config_t io_conf_rst = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_RST_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_rst);
}

// ===== KHỞI TẠO OLED =====
void oled_init(void) {
    oled_reset();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    oled_write_cmd(0xAE);
    oled_write_cmd(0xD5); oled_write_cmd(0x80);
    oled_write_cmd(0xA8); oled_write_cmd(0x3F);
    oled_write_cmd(0xD3); oled_write_cmd(0x00);
    oled_write_cmd(0x40);
    oled_write_cmd(0x8D); oled_write_cmd(0x14);
    oled_write_cmd(0x20); oled_write_cmd(0x00);
    oled_write_cmd(0xA1);
    oled_write_cmd(0xC8);
    oled_write_cmd(0xDA); oled_write_cmd(0x12);
    oled_write_cmd(0x81); oled_write_cmd(0xCF);
    oled_write_cmd(0xD9); oled_write_cmd(0xF1);
    oled_write_cmd(0xDB); oled_write_cmd(0x40);
    oled_write_cmd(0xA4);
    oled_write_cmd(0xA6);
    oled_write_cmd(0x2E);
    oled_write_cmd(0xAF);
    
    vTaskDelay(pdMS_TO_TICKS(100));
}

// ===== XÓA MÀN HÌNH =====
void oled_clear(void) {
    uint8_t clear[128] = {0};
    for (int page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
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

// ===== VẼ KÝ TỰ NHỎ =====
void oled_draw_small_char(uint8_t page, uint8_t col, uint8_t c) {
    if (c >= '0' && c <= '9') c = c - '0';
    else if (c == ':') c = 10;
    else return;
    
    oled_write_cmd(0xB0 + page);
    oled_write_cmd(0x00 + (col & 0x0F));
    oled_write_cmd(0x10 + (col >> 4));
    
    for (int i = 0; i < 8; i++) {
        uint8_t byte = font_8x8[c][i];
        oled_write_data(&byte, 1);
    }
}

// ===== HIỂN THỊ POMODORO TIMER =====
void oled_display_pomodoro(void) {
    uint8_t minutes = pomodoro.time_left / 60;
    uint8_t seconds = pomodoro.time_left % 60;
    
    int offset = 30;
    
    oled_draw_small_char(5, offset, minutes / 10 + '0');
    oled_draw_small_char(5, offset + 8, minutes % 10 + '0');
    oled_draw_small_char(5, offset + 16, ':');
    oled_draw_small_char(5, offset + 24, seconds / 10 + '0');
    oled_draw_small_char(5, offset + 32, seconds % 10 + '0');
    
    oled_write_cmd(0xB0 + 5);
    oled_write_cmd(0x00 + ((offset + 45) & 0x0F));
    oled_write_cmd(0x10 + ((offset + 45) >> 4));
    
    if (pomodoro.state == POMODORO_WORK) {
        uint8_t w[] = {0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00};
        oled_write_data(w, 6);
    } else if (pomodoro.state == POMODORO_BREAK || pomodoro.state == POMODORO_LONG_BREAK) {
        uint8_t b[] = {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00};
        oled_write_data(b, 6);
    }
    
    if (pomodoro.pomodoro_count < 10) {
        oled_draw_small_char(5, offset + 55, pomodoro.pomodoro_count + '0');
    }
}

// ===== HIỂN THỊ CHỈ GIỜ =====
void oled_display_time_only(rtc_time_t *time) {
    int add = 15;
    oled_draw_large_char(2, add, time->hours / 10 + '0');
    oled_draw_large_char(2, 16 + add, time->hours % 10 + '0');
    oled_draw_large_char(2, 28 + add, ':');
    oled_draw_large_char(2, 40 + add, time->minutes / 10 + '0');
    oled_draw_large_char(2, 52 + add, time->minutes % 10 + '0');
    oled_draw_large_char(2, 64 + add, ':');
    oled_draw_large_char(2, 76 + add, time->seconds / 10 + '0');
    oled_draw_large_char(2, 88 + add, time->seconds % 10 + '0');
}

// ===== HIỂN THỊ CHỈ NGÀY =====
void oled_display_date_only(rtc_time_t *time) {
    int add = 25;
    oled_draw_large_char(2, add, time->date / 10 + '0');
    oled_draw_large_char(2, 16 + add, time->date % 10 + '0');
    oled_draw_large_char(2, 32 + add, '/');
    oled_draw_large_char(2, 48 + add, time->month / 10 + '0');
    oled_draw_large_char(2, 64 + add, time->month % 10 + '0');
    
    int offset = 30;
    uint16_t year = time->year;
    oled_draw_large_char(5, offset, (year / 1000) + '0');
    oled_draw_large_char(5, 16 + offset, ((year / 100) % 10) + '0');
    oled_draw_large_char(5, 32 + offset, ((year / 10) % 10) + '0');
    oled_draw_large_char(5, 48 + offset, (year % 10) + '0');
}

// ===== ĐỌC THỜI GIAN DS3231 =====
esp_err_t ds3231_get_time(rtc_time_t *time) {
    uint8_t reg = 0x00;
    uint8_t data[7];
    
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, DS3231_ADDR,
                                                &reg, 1, data, 7, 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) return err;
    
    time->seconds = bcd_to_dec(data[0] & 0x7F);
    time->minutes = bcd_to_dec(data[1] & 0x7F);
    time->hours   = bcd_to_dec(data[2] & 0x3F);
    time->day     = bcd_to_dec(data[3] & 0x07);
    time->date    = bcd_to_dec(data[4] & 0x3F);
    time->month   = bcd_to_dec(data[5] & 0x1F);
    time->year    = bcd_to_dec(data[6]) + 2000;
    
    return ESP_OK;
}

// ===== XỬ LÝ POMODORO =====
void pomodoro_start_stop(void) {
    pomodoro.is_running = !pomodoro.is_running;
    ESP_LOGI(TAG, "Pomodoro %s", pomodoro.is_running ? "Started" : "Paused");
}

void pomodoro_reset(void) {
    pomodoro.is_running = false;
    pomodoro.state = POMODORO_WORK;
    pomodoro.time_left = POMODORO_WORK_DURATION;
    ESP_LOGI(TAG, "Pomodoro Reset");
}

void pomodoro_tick(void) {
    if (!pomodoro.is_running) return;
    
    if (pomodoro.time_left > 0) {
        pomodoro.time_left--;
    } else {
        if (pomodoro.state == POMODORO_WORK) {
            pomodoro.pomodoro_count++;
            
            if (pomodoro.pomodoro_count % 4 == 0) {
                pomodoro.state = POMODORO_LONG_BREAK;
                pomodoro.time_left = POMODORO_LONG_BREAK_DURATION;
            } else {
                pomodoro.state = POMODORO_BREAK;
                pomodoro.time_left = POMODORO_BREAK_DURATION;
            }
        } else {
            pomodoro.state = POMODORO_WORK;
            pomodoro.time_left = POMODORO_WORK_DURATION;
        }
        pomodoro.is_running = false;
    }
}

// ===== WEB SERVER HANDLERS =====

// HTML trang chủ với giao diện đẹp
static esp_err_t root_handler(httpd_req_t *req) {
    const char* html_part1 = 
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32 Pomodoro Timer</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#1a1a2e 0%,#16213e 100%);"
        "color:#eee;min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}"
        ".container{max-width:600px;width:100%;background:#16213e;padding:40px;border-radius:20px;"
        "box-shadow:0 10px 40px rgba(0,0,0,0.5);animation:fadeIn 0.5s ease-in}"
        "@keyframes fadeIn{from{opacity:0;transform:translateY(-20px)}to{opacity:1;transform:translateY(0)}}"
        "h1{color:#e94560;margin-bottom:40px;font-size:2.5em;text-align:center;"
        "text-shadow:2px 2px 8px rgba(233,69,96,0.3)}"
        ".clock-section{text-align:center;margin-bottom:40px;padding:20px;"
        "background:rgba(15,52,96,0.3);border-radius:15px}"
        ".time{font-size:72px;font-weight:bold;color:#e94560;"
        "text-shadow:0 0 20px rgba(233,69,96,0.5);margin:10px 0;"
        "font-family:'Courier New',monospace;letter-spacing:5px}"
        ".date{font-size:28px;color:#aaa;margin:10px 0}"
        ".pomodoro{margin:30px 0;padding:30px;"
        "background:linear-gradient(135deg,#0f3460 0%,#16213e 100%);"
        "border-radius:15px;border:2px solid rgba(233,69,96,0.2)}"
        ".pomo-state{font-size:24px;color:#aaa;margin:15px 0;"
        "text-transform:uppercase;letter-spacing:2px}"
        ".pomo-timer{font-size:64px;font-weight:bold;color:#e94560;margin:20px 0;"
        "font-family:'Courier New',monospace;text-shadow:0 0 30px rgba(233,69,96,0.6);"
        "animation:pulse 2s ease-in-out infinite}"
        "@keyframes pulse{0%,100%{transform:scale(1)}50%{transform:scale(1.02)}}"
        ".pomo-count{font-size:20px;color:#bbb;margin:15px 0}"
        ".status{margin:20px 0;padding:15px;border-radius:10px;font-size:18px;"
        "font-weight:bold;text-align:center;text-transform:uppercase;letter-spacing:1px;transition:all 0.3s}";
    
    const char* html_part2 = 
        ".running{background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);color:white;"
        "box-shadow:0 5px 20px rgba(39,174,96,0.4);animation:statusPulse 1.5s ease-in-out infinite}"
        ".paused{background:linear-gradient(135deg,#e74c3c 0%,#c0392b 100%);color:white;"
        "box-shadow:0 5px 20px rgba(231,76,60,0.4)}"
        "@keyframes statusPulse{0%,100%{box-shadow:0 5px 20px rgba(39,174,96,0.4)}"
        "50%{box-shadow:0 5px 30px rgba(39,174,96,0.8)}}"
        ".button-group{display:flex;gap:15px;justify-content:center;flex-wrap:wrap;margin-top:30px}"
        ".btn{padding:18px 45px;font-size:18px;border:none;border-radius:12px;cursor:pointer;"
        "transition:all 0.3s;font-weight:bold;text-transform:uppercase;letter-spacing:1px;"
        "box-shadow:0 5px 15px rgba(0,0,0,0.3)}"
        ".btn:hover{transform:translateY(-3px);box-shadow:0 8px 20px rgba(0,0,0,0.4)}"
        ".btn:active{transform:translateY(-1px)}"
        ".btn-start{background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);color:white}"
        ".btn-start:hover{background:linear-gradient(135deg,#2ecc71 0%,#27ae60 100%)}"
        ".btn-reset{background:linear-gradient(135deg,#f39c12 0%,#e67e22 100%);color:white}"
        ".btn-reset:hover{background:linear-gradient(135deg,#e67e22 0%,#f39c12 100%)}"
        ".connection-status{text-align:center;margin-top:20px;padding:10px;font-size:14px;"
        "color:#888;border-top:1px solid rgba(255,255,255,0.1)}"
        ".connected{color:#2ecc71}"
        "@media (max-width:600px){.container{padding:25px}h1{font-size:2em}"
        ".time{font-size:48px}.date{font-size:20px}.pomo-timer{font-size:48px}"
        ".btn{padding:15px 30px;font-size:16px}}"
        "</style></head><body><div class='container'>"
        "<h1>⏰ Pomodoro Timer</h1><div class='clock-section'>"
        "<div class='time' id='time'>--:--:--</div>"
        "<div class='date' id='date'>--/--/----</div></div>"
        "<div class='pomodoro'><div class='pomo-state' id='state'>Ready to Work</div>"
        "<div class='pomo-timer' id='timer'>25:00</div>"
        "<div class='pomo-count' id='count'>🍅 Pomodoros: 0</div>"
        "<div class='status' id='status'>Paused</div></div>"
        "<div class='button-group'>"
        "<button class='btn btn-start' onclick='startStop()'>▶️ Start/Stop</button>"
        "<button class='btn btn-reset' onclick='reset()'>🔄 Reset</button></div>"
        "<div class='connection-status'><span id='connection' class='connected'>"
        "● Connected to ESP32</span></div></div>";
    
    const char* html_part3 = 
        "<script>"
        "function updateData(){"
        "fetch('/data').then(r=>r.json()).then(d=>{"
        "document.getElementById('time').innerText=d.time;"
        "document.getElementById('date').innerText=d.date;"
        "document.getElementById('timer').innerText=d.timer;"
        "document.getElementById('state').innerText=d.state;"
        "document.getElementById('count').innerText='🍅 Pomodoros: '+d.count;"
        "let s=document.getElementById('status');"
        "if(d.running){s.innerText='⏯ Running';s.className='status running';}"
        "else{s.innerText='⏸ Paused';s.className='status paused';}"
        "}).catch(()=>{document.getElementById('connection').className='';})}"
        "function startStop(){fetch('/start').then(()=>setTimeout(updateData,100));}"
        "function reset(){fetch('/reset').then(()=>setTimeout(updateData,100));}"
        "setInterval(updateData,1000);updateData();"
        "</script></body></html>";
    
    // Gửi từng phần
    httpd_resp_send_chunk(req, html_part1, strlen(html_part1));
    httpd_resp_send_chunk(req, html_part2, strlen(html_part2));
    httpd_resp_send_chunk(req, html_part3, strlen(html_part3));
    httpd_resp_send_chunk(req, NULL, 0); // Kết thúc
    
    return ESP_OK;
}

// API lấy dữ liệu
static esp_err_t data_handler(httpd_req_t *req) {
    char json[350];
    
    const char* state_str = "Ready to Work";
    const char* emoji = "💪";
    
    if (pomodoro.state == POMODORO_WORK) {
        state_str = "Work Time";
        emoji = "💪";
    } else if (pomodoro.state == POMODORO_BREAK) {
        state_str = "Short Break";
        emoji = "☕";
    } else if (pomodoro.state == POMODORO_LONG_BREAK) {
        state_str = "Long Break";
        emoji = "🎉";
    }
    
    snprintf(json, sizeof(json),
        "{\"time\":\"%02d:%02d:%02d\","
        "\"date\":\"%02d/%02d/%04d\","
        "\"timer\":\"%02d:%02d\","
        "\"state\":\"%s %s\","
        "\"running\":%s,"
        "\"count\":%d}",
        current_time.hours, current_time.minutes, current_time.seconds,
        current_time.date, current_time.month, current_time.year,
        pomodoro.time_left / 60, pomodoro.time_left % 60,
        emoji, state_str,
        pomodoro.is_running ? "true" : "false",
        pomodoro.pomodoro_count
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// API Start/Stop
static esp_err_t start_handler(httpd_req_t *req) {
    pomodoro_start_stop();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

// API Reset
static esp_err_t reset_handler(httpd_req_t *req) {
    pomodoro_reset();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

// ===== KHỞI TẠO WEB SERVER =====
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 8;
    
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
        httpd_uri_t data = {.uri = "/data", .method = HTTP_GET, .handler = data_handler};
        httpd_uri_t start = {.uri = "/start", .method = HTTP_GET, .handler = start_handler};
        httpd_uri_t reset = {.uri = "/reset", .method = HTTP_GET, .handler = reset_handler};
        
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &data);
        httpd_register_uri_handler(server, &start);
        httpd_register_uri_handler(server, &reset);
        
        return server;
    }
    
    ESP_LOGE(TAG, "Error starting server!");
    return NULL;
}

// ===== WIFI EVENT HANDLER =====
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        start_webserver();
    }
}

// ===== KHỞI TẠO WIFI =====
void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                    &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                    &wifi_event_handler, NULL, &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi initialization finished. Connecting to %s...", WIFI_SSID);
}

// ===== TASK NÚT NHẤN =====
void button_task(void *pvParameter) {
    bool btn_start_last = true;
    bool btn_reset_last = true;
    
    while (1) {
        bool btn_start = gpio_get_level(BTN_START_STOP_GPIO);
        bool btn_reset = gpio_get_level(BTN_RESET_GPIO);
        
        if (!btn_start && btn_start_last) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (!gpio_get_level(BTN_START_STOP_GPIO)) {
                pomodoro_start_stop();
            }
        }
        
        if (!btn_reset && btn_reset_last) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (!gpio_get_level(BTN_RESET_GPIO)) {
                pomodoro_reset();
            }
        }
        
        btn_start_last = btn_start;
        btn_reset_last = btn_reset;
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ===== TASK HIỂN THỊ =====
void display_task(void *pvParameter) {
    bool show_time = true;
    uint8_t counter = 0;
    
    while (1) {
        if (ds3231_get_time(&current_time) == ESP_OK) {
            oled_clear();
            
            if (show_time) {
                oled_display_time_only(&current_time);
                oled_display_pomodoro();
            } else {
                oled_display_date_only(&current_time);
            }
            
            counter++;
            if (counter >= 3) {
                counter = 0;
                show_time = !show_time;
            }
        }
        
        pomodoro_tick();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ===== MAIN =====
void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32-C3 RTC + OLED + Pomodoro + Web");
    ESP_LOGI(TAG, "========================================");
    
    // Khởi tạo NVS (cần cho WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Khởi tạo I2C và DS3231
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized");
    
    // Khởi tạo SPI và OLED
    spi_master_init();
    oled_init();
    oled_clear();
    
    // Khởi tạo nút nhấn
    button_init();
    
    // Khởi tạo Pomodoro
    pomodoro_reset();
    
    // Khởi tạo WiFi
    wifi_init_sta();
    
    // Tạo tasks
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "System started successfully!");
}
