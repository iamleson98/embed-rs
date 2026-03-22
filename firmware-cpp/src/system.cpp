/**
 * System Configuration Implementation (C++)
 */

#include "system.hpp"
#include "stm32f103.hpp"

namespace system {

static volatile uint32_t g_tick_count = 0;

void init_clock() {
    using namespace stm32;

    // Enable HSE (8 MHz external crystal)
    RCC->CR |= rcc::CR_HSEON;
    while (!(RCC->CR & rcc::CR_HSERDY)) {}

    // Configure PLL: HSE * 9 = 72 MHz
    RCC->CFGR |= rcc::CFGR_PLLSRC | rcc::CFGR_PLLMUL9;

    // Enable PLL
    RCC->CR |= rcc::CR_PLLON;
    while (!(RCC->CR & rcc::CR_PLLRDY)) {}

    // Flash: 2 wait states for 72 MHz
    *reinterpret_cast<volatile uint32_t*>(0x40022000) = 0x12; // FLASH->ACR

    // Switch system clock to PLL
    RCC->CFGR |= rcc::CFGR_SW_PLL;
    while ((RCC->CFGR & rcc::CFGR_SWS_PLL) != rcc::CFGR_SWS_PLL) {}

    // Configure SysTick for 1ms interrupts
    SysTick->LOAD = 72000 - 1;  // 72 MHz / 72000 = 1 kHz
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07;  // Enable, interrupt, use processor clock
}

uint32_t get_tick() {
    return g_tick_count;
}

void delay_ms(uint32_t ms) {
    uint32_t start = g_tick_count;
    while ((g_tick_count - start) < ms) {}
}

} // namespace system

extern "C" void SysTick_Handler() {
    system::g_tick_count++;
}
