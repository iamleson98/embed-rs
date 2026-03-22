/**
 * System Initialization — Clock Configuration
 * Configures HSE + PLL to run at 72 MHz, sets up flash latency and bus prescalers.
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

extern volatile uint32_t g_ticks;      /* SysTick millisecond counter */
extern uint32_t SystemCoreClock;

void SystemInit(void);
void delay_ms(uint32_t ms);
uint32_t get_tick(void);

#endif /* SYSTEM_H */
