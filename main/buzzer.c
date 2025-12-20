#include "buzzer.h"

void buzzer_init(void) {
    // Cấu hình timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Cấu hình channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0, // Bắt đầu ở trạng thái tắt
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void buzzer_on(uint32_t frequency)
{
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void buzzer_off(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}


void buzzer_beep(uint32_t frequency, uint32_t duration_ms)
{
    buzzer_on(frequency);
    vTaskDelay(duration_ms / portTICK_PERIOD_MS);
    buzzer_off();
}

void buzzer_alert(void) {
    for (int i = 0; i < 3; i++) {
        buzzer_beep(2000, 200);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void buzzer_beep_5s (void) {
    uint32_t total_duration = 5000;
    uint32_t beep_on = 150;
    uint32_t beep_short_pause = 100;  // Khoảng cách giữa 2 tiếng tít
    uint32_t beep_long_pause = 400;   // Khoảng cách giữa các cặp
    uint32_t frequency = 2500;
    
    uint32_t elapsed = 0;
    
    while (elapsed < total_duration) {
        // Tít đầu tiên
        buzzer_on(frequency);
        vTaskDelay(beep_on / portTICK_PERIOD_MS);
        buzzer_off();
        vTaskDelay(beep_short_pause / portTICK_PERIOD_MS);
        
        // Tít thứ hai
        buzzer_on(frequency);
        vTaskDelay(beep_on / portTICK_PERIOD_MS);
        buzzer_off();
        vTaskDelay(beep_long_pause / portTICK_PERIOD_MS);
        
        elapsed += (beep_on * 2 + beep_short_pause + beep_long_pause);
    }
    
    buzzer_off();
}