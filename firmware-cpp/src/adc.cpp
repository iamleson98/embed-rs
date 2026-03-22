/**
 * ADC Driver Implementation (C++)
 */

#include "adc.hpp"
#include "stm32f103.hpp"

namespace adc {

void init() {
    using namespace stm32;

    // Enable ADC1 clock
    RCC->APB2ENR |= rcc::APB2ENR_ADC1EN | rcc::APB2ENR_IOPAEN;

    // Configure PA0 as analog input
    GPIOA->CRL &= ~(0xF << 0);

    // ADC configuration
    ADC1->CR2 = adc::CR2_ADON;  // Power on
    for (volatile int i = 0; i < 10000; i++) {}  // Stabilization delay

    // Calibrate
    ADC1->CR2 |= adc::CR2_CAL;
    while (ADC1->CR2 & adc::CR2_CAL) {}

    // Enable temperature sensor
    ADC1->CR2 |= adc::CR2_TSVREFE;

    // Set sample time for all channels (239.5 cycles)
    ADC1->SMPR1 = 0x00FFFFFF;
    ADC1->SMPR2 = 0x3FFFFFFF;
}

uint16_t read(uint8_t channel) {
    using namespace stm32;

    // Select channel
    ADC1->SQR3 = channel;

    // Start conversion
    ADC1->CR2 |= adc::CR2_SWSTART;

    // Wait for end of conversion
    while (!(ADC1->SR & adc::SR_EOC)) {}

    return static_cast<uint16_t>(ADC1->DR);
}

int32_t read_temperature_mc() {
    // Read internal temperature sensor (channel 16)
    uint16_t raw = read(16);

    // STM32F103 temperature formula:
    // T(°C) = (V25 - Vsense) / Avg_Slope + 25
    // Where: V25 ≈ 1.43V, Avg_Slope ≈ 4.3mV/°C
    // Simplified: T(m°C) ≈ (1430 - (raw * 3300 / 4096)) * 1000 / 4.3 + 25000

    int32_t v_sense = (static_cast<int32_t>(raw) * 3300) / 4096;  // mV
    int32_t temp_mc = ((1430 - v_sense) * 232) + 25000;  // Simplified calculation

    return temp_mc;
}

} // namespace adc
