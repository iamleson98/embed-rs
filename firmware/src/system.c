/**
 * System Initialization — Clock Configuration
 * HSE 8 MHz -> PLL x9 -> 72 MHz SYSCLK
 * APB1 = 36 MHz, APB2 = 72 MHz
 */

#include "stm32f103.h"
#include "system.h"

volatile uint32_t g_ticks = 0;
uint32_t SystemCoreClock = 72000000UL;

void SystemInit(void) {
    /* Enable HSE (external 8 MHz crystal) */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    /* Flash: 2 wait states for 72 MHz, enable prefetch */
    FLASH->ACR = (2UL << FLASH_ACR_LATENCY_Pos) | FLASH_ACR_PRFTBE;

    /*
     * PLL configuration:
     *   PLLSRC = HSE
     *   PLLMUL = x9   -> 8 MHz * 9 = 72 MHz
     *   AHB prescaler  = /1  (72 MHz)
     *   APB1 prescaler = /2  (36 MHz max)
     *   APB2 prescaler = /1  (72 MHz)
     *   ADC prescaler  = /6  (12 MHz)
     */
    RCC->CFGR = RCC_CFGR_PLLSRC                        /* HSE as PLL input     */
              | (0x7UL << RCC_CFGR_PLLMULL_Pos)         /* PLL x9               */
              | (0x0UL << RCC_CFGR_HPRE_Pos)            /* AHB  = SYSCLK / 1    */
              | (0x4UL << RCC_CFGR_PPRE1_Pos)           /* APB1 = AHB / 2       */
              | (0x0UL << RCC_CFGR_PPRE2_Pos)           /* APB2 = AHB / 1       */
              | RCC_CFGR_ADCPRE_DIV6;                   /* ADC  = APB2 / 6      */

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    /* Switch SYSCLK to PLL */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL)
        ;

    /* Configure SysTick for 1 ms interrupt */
    SysTick->LOAD = (SystemCoreClock / 1000UL) - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SYSTICK_CTRL_CLKSOURCE
                   | SYSTICK_CTRL_TICKINT
                   | SYSTICK_CTRL_ENABLE;
}

void SysTick_Handler(void) {
    g_ticks++;
}

uint32_t get_tick(void) {
    return g_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms)
        ;
}
