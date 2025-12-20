#include "touch_sensor.h"

static volatile uint8_t touch_count = 0;
static volatile uint32_t last_interrupt_time = 0;

static volatile uint8_t pomo_state = 0;
static volatile uint32_t last_change = 0;


static void IRAM_ATTR touch_isr_handler () {
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if ((now - last_interrupt_time) > DEBOUNCE_TIME_MS) {
        touch_count++;
        if (touch_count > MODE) {
            touch_count = 0;
        }
        last_interrupt_time = now;
    }
}

static void IRAM_ATTR pomo_isr_handler () {
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if ((now - last_change) > DEBOUNCE_TIME_MS) {
        pomo_state++;
        if (pomo_state > 1) {
            pomo_state = 0;
        }
        last_change = now;
    }
}

void touch_init () {
    gpio_config_t io_touch = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TOUCH_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_touch);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCH_PIN, touch_isr_handler, NULL);
}

uint8_t touch_get_mode() {
    return touch_count;
}
void touch_set_mode(uint8_t mode) {
    if (mode <= MODE) {
        touch_count = mode;
    }
}

void change_pomo_init () {
    gpio_config_t io_change_pomo = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << CHANGE_POMO_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_change_pomo);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(CHANGE_POMO_PIN, pomo_isr_handler, NULL);
}

uint8_t pomo_get_state() {
    return pomo_state;
}