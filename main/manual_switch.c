#include "manual_switch.h"

static const char* TAG = "SWITCH MODE";
static int last_mode = -1;

static void mode_switch(int mode) {
    switch (mode)
    {
    case 0:
        white_led();
        break;
    case 1:
        yellow_led();
        break;
    case 2:
        blue_led();
        break;
    default:
        white_led();
        break;
    }
}

void manual_switch() {
    int mode = touch_get_mode();
    if (mode != last_mode){
        ESP_LOGI(TAG, "Mode: %d", mode);
        last_mode = mode;
        mode_switch(mode);
    }
}