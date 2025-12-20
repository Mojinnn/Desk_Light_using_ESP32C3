#include "main.h"

void buzzer_init(void);
void buzzer_on(uint32_t frequency);
void buzzer_off(void);
void buzzer_beep(uint32_t frequency, uint32_t duration_ms);
void buzzer_alert(void);