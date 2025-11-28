#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h" 
#include "driver/rmt_tx.h"
#include "led_strip.h"

#define LED_PIN             8
#define LED_NUM             9
#define BUTTON_PIN          0
#define TOUCH_PIN           2
#define DEBOUNCE_TIME_MS    50

#endif