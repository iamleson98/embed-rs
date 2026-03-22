/**
 * Timer/PWM Driver (C++)
 */

#pragma once

#include <cstdint>

namespace timer {

/**
 * Initialize PWM output on TIM3 CH1
 * @param freq_hz PWM frequency in Hz
 */
void pwm_init(uint32_t freq_hz);

/**
 * Set PWM duty cycle
 * @param duty Duty cycle (0-100%)
 */
void pwm_set_duty(uint8_t duty);

/**
 * Initialize sampling timer (TIM2) with interrupt
 * @param rate_hz Sampling rate in Hz
 */
void sampling_timer_init(uint32_t rate_hz);

} // namespace timer
