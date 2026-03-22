/**
 * Timer / PWM Driver
 *
 * TIM3 CH1 -> PA6 (PWM output for fan control)
 *   - PWM Mode 1, configurable frequency
 *
 * TIM2 -> Periodic interrupt for ADC sampling
 */

#include "timer.h"
#include "stm32f103.h"
#include "gpio.h"

static uint32_t pwm_period = 0;

void pwm_init(uint32_t freq_hz) {
    /* Enable TIM3 clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* PA6 = TIM3_CH1 (AF push-pull) */
    gpio_set_mode(GPIOA, 6, PIN_MODE_AF_PP_50MHZ);

    /* TIM3 runs at APB1 * 2 = 72 MHz (APB1 prescaler != 1) */
    uint32_t timer_clk = 72000000UL;

    /* Choose prescaler to keep period in 16-bit range */
    uint32_t psc = 0;
    uint32_t arr = timer_clk / freq_hz;
    while (arr > 65535) {
        psc++;
        arr = timer_clk / ((psc + 1) * freq_hz);
    }

    TIM3->PSC  = psc;
    TIM3->ARR  = arr - 1;
    TIM3->CCR1 = 0;        /* Start at 0% duty */

    pwm_period = arr;

    /* PWM Mode 1 on CH1, preload enable */
    TIM3->CCMR1 = TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;

    /* Enable CH1 output */
    TIM3->CCER = TIM_CCER_CC1E;

    /* Auto-reload preload, start counter */
    TIM3->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;

    /* Force update to load registers */
    TIM3->EGR = TIM_EGR_UG;
}

void pwm_set_duty(uint8_t percent) {
    if (percent > 100) percent = 100;
    TIM3->CCR1 = (pwm_period * (uint32_t)percent) / 100;
}

void sampling_timer_init(uint32_t sample_rate_hz) {
    /* Enable TIM2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* TIM2 runs at 72 MHz */
    uint32_t timer_clk = 72000000UL;

    /* Prescale to 10 kHz, then divide further */
    uint32_t psc = 7199;   /* 72 MHz / 7200 = 10 kHz */
    uint32_t arr = (10000UL / sample_rate_hz) - 1;

    TIM2->PSC  = psc;
    TIM2->ARR  = arr;
    TIM2->CR1  = TIM_CR1_ARPE;

    /* Enable update interrupt */
    TIM2->DIER = TIM_DIER_UIE;

    /* Clear pending flag and start */
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->CR1 |= TIM_CR1_CEN;

    /* Enable TIM2 IRQ in NVIC */
    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);
}
