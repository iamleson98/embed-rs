/**
 * System Configuration (C++)
 * Clock setup and timing functions
 */

#pragma once

#include <cstdint>

namespace system {

/**
 * Initialize system clock to 72 MHz (HSE + PLL)
 */
void init_clock();

/**
 * Get current system tick count in milliseconds
 */
uint32_t get_tick();

/**
 * Blocking delay in milliseconds
 */
void delay_ms(uint32_t ms);

/**
 * SysTick interrupt handler - must be defined externally
 */
extern "C" void SysTick_Handler();

} // namespace system
