/**
 * Timer / PWM Driver Interface
 * TIM3 CH1 on PB4 (fan PWM output) — requires AFIO remap
 * TIM2 used for periodic sampling interrupt
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void pwm_init(uint32_t freq_hz);
void pwm_set_duty(uint8_t percent);

void sampling_timer_init(uint32_t sample_rate_hz);

#endif /* TIMER_H */
