#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN 8
#define BUTTON_PIN  0 



static volatile bool led_status = false ; 

// ===== Interupt handle ============== // 
static void IRAM_ATTR button_isr_handler(void *arg) {
    led_status = !led_status ; 
}
void Configure(void) {
        //============== configure LED ===================// 
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL<LED_PIN),
        .mode = GPIO_MODE_OUTPUT 
    } ; 
    gpio_config(&led_conf) ; 
    // ============== config button ===================// 
    gpio_config_t btn_conf = {
         .pin_bit_mask = (1ULL<<BUTTON_PIN), 
         .mode = GPIO_MODE_INPUT ,
         .pull_up_en = GPIO_PULLUP_ENABLE,
         .pull_down_en = GPIO_PULLDOWN_DISABLE, 
         .intr_type = GPIO_INTR_NEGEDGE 
    } ; 
    gpio_config(&btn_conf) ; 
    // ============== ISR register =================== // 
    gpio_install_isr_service(0); 
    gpio_isr_handler_add(BUTTON_PIN,button_isr_handler,NULL) ; 
}
