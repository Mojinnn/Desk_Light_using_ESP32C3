#include "change_pomo.h"

int last_pomo = -1;

static void pomo_mode(int mode) {
    switch (mode)
    {
    case 0:
        pomodoro_reset();
        break;
    case 1:
        pomodoro_start_stop();
        break;
    default:
        pomodoro_reset();
        break;
    }
}

void manual_pomo() {
    int mode = pomo_get_state();
    if (mode != last_pomo){
        last_pomo = mode;
        pomo_mode(mode);
    }
}

static void manual_pomo_task(void *arg) {
    while (1)
    {
        manual_pomo();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}

void manual_pomo_start(void) {
    xTaskCreate(manual_pomo_task, "Change Pomodoro", 2048, NULL, 5, NULL);
}