#include "main.h"
#include "manual_switch.h"

void app_main(void)
{
    led_init();
    touch_init();

    while (1)
    {
        manual_switch();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
