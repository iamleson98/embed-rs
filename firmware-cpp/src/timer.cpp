/**
 * Timer/PWM Driver Implementation (C++)
 */

#include "timer.hpp"
#include "stm32f103.hpp"

namespace timer {

static uint16_t g_pwm_period = 0;

void pwm_init(uint32_t freq_hz) {
    using namespace stm32;

    // Enable TIM3 and GPIOA clocks
    RCC->APB1ENR |= rcc::APB1ENR_TIM3EN;
    RCC->APB2ENR |= rcc::APB2ENR_IOPAEN;

    // Configure PA6 as alternate function push-pull (TIM3_CH1)
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |= (0xB << 24);  // 50MHz, AF push-pull

    // Configure TIM3
    // APB1 clock = 36 MHz, but timer clock = 72 MHz (due to prescaler multiplier)
    uint32_t timer_clock = 72000000;
    uint16_t prescaler = 0;
    uint16_t period = timer_clock / freq_hz - 1;

    // Adjust if period is too large
    while (period > 65535) {
        prescaler++;
        period = (timer_clock / (prescaler + 1)) / freq_hz - 1;
    }

    g_pwm_period = period;

    TIM3->PSC = prescaler;
    TIM3->ARR = period;
    TIM3->CCR1 = 0;  // Start with 0% duty

    // PWM mode 1 on channel 1
    TIM3->CCMR1 = (6 << 4) | (1 << 3);  // OC1M = PWM mode 1, OC1PE = preload enable
    TIM3->CCER = tim::CCER_CC1E;  // Enable channel 1 output

    // Enable timer
    TIM3->CR1 = tim::CR1_CEN;
}

void pwm_set_duty(uint8_t duty) {
    if (duty > 100) duty = 100;

    uint32_t ccr = (static_cast<uint32_t>(g_pwm_period) * duty) / 100;
    stm32::TIM3->CCR1 = ccr;
}

void sampling_timer_init(uint32_t rate_hz) {
    using namespace stm32;

    // Enable TIM2 clock
    RCC->APB1ENR |= rcc::APB1ENR_TIM2EN;

    // Configure TIM2 for periodic interrupt
    uint32_t timer_clock = 72000000;
    uint16_t prescaler = 7199;  // 72MHz / 7200 = 10kHz
    uint16_t period = (10000 / rate_hz) - 1;

    TIM2->PSC = prescaler;
    TIM2->ARR = period;
    TIM2->DIER = tim::DIER_UIE;  // Update interrupt enable

    // Enable TIM2 interrupt in NVIC
    *reinterpret_cast<volatile uint32_t*>(0xE000E100) |= (1 << 28);  // NVIC_ISER0

    // Start timer
    TIM2->CR1 = tim::CR1_CEN;
}

} // namespace timer
