/**
 * STM32F103 Register Definitions (C++)
 * Modern C++17 type-safe peripheral access
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace stm32 {

// Base addresses
constexpr uintptr_t PERIPH_BASE      = 0x40000000UL;
constexpr uintptr_t APB1_BASE        = PERIPH_BASE;
constexpr uintptr_t APB2_BASE        = PERIPH_BASE + 0x10000UL;
constexpr uintptr_t AHB_BASE         = PERIPH_BASE + 0x20000UL;
constexpr uintptr_t FLASH_BASE       = 0x08000000UL;
constexpr uintptr_t SRAM_BASE        = 0x20000000UL;

// RCC (Reset and Clock Control)
struct RCC_Regs {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
};

// GPIO
struct GPIO_Regs {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
};

// USART
struct USART_Regs {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
};

// TIM (Timer)
struct TIM_Regs {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
};

// ADC
struct ADC_Regs {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
};

// IWDG (Independent Watchdog)
struct IWDG_Regs {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
};

// SCB (System Control Block)
struct SCB_Regs {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t  SHP[12];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t DFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
};

// SysTick
struct SysTick_Regs {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
};

// Peripheral instances
inline RCC_Regs* const RCC        = reinterpret_cast<RCC_Regs*>(AHB_BASE + 0x1000);
inline GPIO_Regs* const GPIOA     = reinterpret_cast<GPIO_Regs*>(APB2_BASE + 0x0800);
inline GPIO_Regs* const GPIOB     = reinterpret_cast<GPIO_Regs*>(APB2_BASE + 0x0C00);
inline GPIO_Regs* const GPIOC     = reinterpret_cast<GPIO_Regs*>(APB2_BASE + 0x1000);
inline USART_Regs* const USART1   = reinterpret_cast<USART_Regs*>(APB2_BASE + 0x3800);
inline TIM_Regs* const TIM2       = reinterpret_cast<TIM_Regs*>(APB1_BASE + 0x0000);
inline TIM_Regs* const TIM3       = reinterpret_cast<TIM_Regs*>(APB1_BASE + 0x0400);
inline ADC_Regs* const ADC1       = reinterpret_cast<ADC_Regs*>(APB2_BASE + 0x2400);
inline IWDG_Regs* const IWDG      = reinterpret_cast<IWDG_Regs*>(APB1_BASE + 0x3000);
inline SCB_Regs* const SCB        = reinterpret_cast<SCB_Regs*>(0xE000ED00UL);
inline SysTick_Regs* const SysTick = reinterpret_cast<SysTick_Regs*>(0xE000E010UL);

// Register bit definitions
namespace rcc {
    constexpr uint32_t CR_HSEON      = (1 << 16);
    constexpr uint32_t CR_HSERDY     = (1 << 17);
    constexpr uint32_t CR_PLLON      = (1 << 24);
    constexpr uint32_t CR_PLLRDY     = (1 << 25);
    constexpr uint32_t CFGR_PLLMUL9  = (7 << 18);
    constexpr uint32_t CFGR_PLLSRC   = (1 << 16);
    constexpr uint32_t CFGR_SW_PLL   = (2 << 0);
    constexpr uint32_t CFGR_SWS_PLL  = (2 << 2);
    constexpr uint32_t APB2ENR_IOPAEN = (1 << 2);
    constexpr uint32_t APB2ENR_IOPCEN = (1 << 4);
    constexpr uint32_t APB2ENR_ADC1EN = (1 << 9);
    constexpr uint32_t APB2ENR_USART1EN = (1 << 14);
    constexpr uint32_t APB1ENR_TIM2EN = (1 << 0);
    constexpr uint32_t APB1ENR_TIM3EN = (1 << 1);
}

namespace usart {
    constexpr uint32_t SR_TXE  = (1 << 7);
    constexpr uint32_t SR_RXNE = (1 << 5);
    constexpr uint32_t CR1_UE  = (1 << 13);
    constexpr uint32_t CR1_TE  = (1 << 3);
    constexpr uint32_t CR1_RE  = (1 << 2);
}

namespace adc {
    constexpr uint32_t CR2_ADON   = (1 << 0);
    constexpr uint32_t CR2_CAL    = (1 << 3);
    constexpr uint32_t CR2_TSVREFE = (1 << 23);
    constexpr uint32_t CR2_SWSTART = (1 << 22);
    constexpr uint32_t SR_EOC     = (1 << 1);
}

namespace tim {
    constexpr uint32_t CR1_CEN    = (1 << 0);
    constexpr uint32_t DIER_UIE   = (1 << 0);
    constexpr uint32_t SR_UIF     = (1 << 0);
    constexpr uint32_t CCER_CC1E  = (1 << 0);
}

namespace iwdg {
    constexpr uint32_t KEY_ENABLE = 0xCCCC;
    constexpr uint32_t KEY_RELOAD = 0xAAAA;
    constexpr uint32_t KEY_ACCESS = 0x5555;
}

namespace scb {
    constexpr uint32_t AIRCR_VECTKEY    = (0x05FA << 16);
    constexpr uint32_t AIRCR_SYSRESETREQ = (1 << 2);
}

} // namespace stm32
