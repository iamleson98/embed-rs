/**
 * ADC Driver (C++)
 */

#pragma once

#include <cstdint>

namespace adc {

/**
 * Initialize ADC1
 */
void init();

/**
 * Read ADC channel
 * @param channel ADC channel number (0-17)
 * @return 12-bit ADC reading (0-4095)
 */
uint16_t read(uint8_t channel);

/**
 * Read internal temperature sensor in milli-degrees Celsius
 */
int32_t read_temperature_mc();

} // namespace adc
