/**
 * STM32F103 Register Definitions — Bare Metal
 * Minimal CMSIS-style register map for STM32F103C8T6 (Blue Pill)
 */

#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

/* ---- Base addresses ---- */
#define PERIPH_BASE       0x40000000UL
#define APB1_BASE         PERIPH_BASE
#define APB2_BASE         (PERIPH_BASE + 0x10000UL)
#define AHB_BASE          (PERIPH_BASE + 0x20000UL)
#define SRAM_BASE         0x20000000UL
#define FLASH_BASE_ADDR   0x08000000UL

/* ---- RCC ---- */
#define RCC_BASE          (AHB_BASE + 0x1000UL)

typedef struct {
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
} RCC_TypeDef;

#define RCC               ((RCC_TypeDef *)RCC_BASE)

/* RCC_CR bits */
#define RCC_CR_HSION      (1UL << 0)
#define RCC_CR_HSIRDY     (1UL << 1)
#define RCC_CR_HSEON      (1UL << 16)
#define RCC_CR_HSERDY     (1UL << 17)
#define RCC_CR_PLLON      (1UL << 24)
#define RCC_CR_PLLRDY     (1UL << 25)

/* RCC_CFGR bits */
#define RCC_CFGR_SW_Pos       0
#define RCC_CFGR_SW_Msk       (0x3UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_HSI       (0x0UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_HSE       (0x1UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_PLL       (0x2UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SWS_Pos      2
#define RCC_CFGR_SWS_Msk      (0x3UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_PLL      (0x2UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_HPRE_Pos     4
#define RCC_CFGR_PPRE1_Pos    8
#define RCC_CFGR_PPRE2_Pos    11
#define RCC_CFGR_PLLSRC       (1UL << 16)
#define RCC_CFGR_PLLXTPRE     (1UL << 17)
#define RCC_CFGR_PLLMULL_Pos  18
#define RCC_CFGR_PLLMULL_Msk  (0xFUL << RCC_CFGR_PLLMULL_Pos)
#define RCC_CFGR_ADCPRE_Pos   14
#define RCC_CFGR_ADCPRE_DIV6  (0x2UL << RCC_CFGR_ADCPRE_Pos)

/* RCC_APB2ENR bits */
#define RCC_APB2ENR_IOPAEN    (1UL << 2)
#define RCC_APB2ENR_IOPBEN    (1UL << 3)
#define RCC_APB2ENR_IOPCEN    (1UL << 4)
#define RCC_APB2ENR_ADC1EN    (1UL << 9)
#define RCC_APB2ENR_TIM1EN    (1UL << 11)
#define RCC_APB2ENR_USART1EN  (1UL << 14)
#define RCC_APB2ENR_AFIOEN    (1UL << 0)

/* RCC_APB1ENR bits */
#define RCC_APB1ENR_TIM2EN    (1UL << 0)
#define RCC_APB1ENR_TIM3EN    (1UL << 1)
#define RCC_APB1ENR_WWDGEN    (1UL << 11)
#define RCC_APB1ENR_USART2EN  (1UL << 17)

/* ---- FLASH ---- */
#define FLASH_R_BASE      (AHB_BASE + 0x2000UL)

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

#define FLASH             ((FLASH_TypeDef *)FLASH_R_BASE)

#define FLASH_ACR_LATENCY_Pos  0
#define FLASH_ACR_PRFTBE       (1UL << 4)

/* ---- GPIO ---- */
#define GPIOA_BASE        (APB2_BASE + 0x0800UL)
#define GPIOB_BASE        (APB2_BASE + 0x0C00UL)
#define GPIOC_BASE        (APB2_BASE + 0x1000UL)

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOA             ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB             ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC             ((GPIO_TypeDef *)GPIOC_BASE)

/* GPIO mode/config values (4 bits per pin in CRL/CRH) */
#define GPIO_MODE_INPUT       0x0UL
#define GPIO_MODE_OUT_10MHZ   0x1UL
#define GPIO_MODE_OUT_2MHZ    0x2UL
#define GPIO_MODE_OUT_50MHZ   0x3UL

#define GPIO_CNF_OUT_PP       (0x0UL << 2)
#define GPIO_CNF_OUT_OD       (0x1UL << 2)
#define GPIO_CNF_AF_PP        (0x2UL << 2)
#define GPIO_CNF_AF_OD        (0x3UL << 2)

#define GPIO_CNF_IN_ANALOG    (0x0UL << 2)
#define GPIO_CNF_IN_FLOAT     (0x1UL << 2)
#define GPIO_CNF_IN_PUPD      (0x2UL << 2)

/* ---- USART ---- */
#define USART1_BASE       (APB2_BASE + 0x3800UL)
#define USART2_BASE       (APB1_BASE + 0x4400UL)

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

#define USART1            ((USART_TypeDef *)USART1_BASE)
#define USART2            ((USART_TypeDef *)USART2_BASE)

/* USART_SR bits */
#define USART_SR_PE       (1UL << 0)
#define USART_SR_TXE      (1UL << 7)
#define USART_SR_TC       (1UL << 6)
#define USART_SR_RXNE     (1UL << 5)
#define USART_SR_ORE      (1UL << 3)

/* USART_CR1 bits */
#define USART_CR1_UE      (1UL << 13)
#define USART_CR1_TE      (1UL << 3)
#define USART_CR1_RE      (1UL << 2)
#define USART_CR1_RXNEIE  (1UL << 5)

/* ---- ADC ---- */
#define ADC1_BASE         (APB2_BASE + 0x2400UL)

typedef struct {
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
} ADC_TypeDef;

#define ADC1              ((ADC_TypeDef *)ADC1_BASE)

/* ADC_SR bits */
#define ADC_SR_EOC        (1UL << 1)

/* ADC_CR1 bits */
#define ADC_CR1_EOCIE     (1UL << 5)
#define ADC_CR1_SCAN      (1UL << 8)

/* ADC_CR2 bits */
#define ADC_CR2_ADON      (1UL << 0)
#define ADC_CR2_CONT      (1UL << 1)
#define ADC_CR2_CAL       (1UL << 2)
#define ADC_CR2_RSTCAL    (1UL << 3)
#define ADC_CR2_SWSTART   (1UL << 22)
#define ADC_CR2_EXTTRIG   (1UL << 20)
#define ADC_CR2_EXTSEL    (0x7UL << 17)
#define ADC_CR2_TSVREFE   (1UL << 23)

/* ---- TIM (General purpose timers) ---- */
#define TIM2_BASE         (APB1_BASE + 0x0000UL)
#define TIM3_BASE         (APB1_BASE + 0x0400UL)

typedef struct {
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
    volatile uint32_t RESERVED1;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t RESERVED2;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

#define TIM2              ((TIM_TypeDef *)TIM2_BASE)
#define TIM3              ((TIM_TypeDef *)TIM3_BASE)

/* TIM_CR1 bits */
#define TIM_CR1_CEN       (1UL << 0)
#define TIM_CR1_ARPE      (1UL << 7)

/* TIM_DIER bits */
#define TIM_DIER_UIE      (1UL << 0)

/* TIM_SR bits */
#define TIM_SR_UIF        (1UL << 0)

/* TIM_EGR */
#define TIM_EGR_UG        (1UL << 0)

/* TIM_CCMR1 (output compare) */
#define TIM_CCMR1_OC1PE   (1UL << 3)
#define TIM_CCMR1_OC1M_Pos  4
#define TIM_CCMR1_OC1M_PWM1 (0x6UL << TIM_CCMR1_OC1M_Pos)
#define TIM_CCMR1_OC2PE   (1UL << 11)
#define TIM_CCMR1_OC2M_Pos  12
#define TIM_CCMR1_OC2M_PWM1 (0x6UL << TIM_CCMR1_OC2M_Pos)

/* TIM_CCER bits */
#define TIM_CCER_CC1E     (1UL << 0)
#define TIM_CCER_CC2E     (1UL << 4)
#define TIM_CCER_CC3E     (1UL << 8)
#define TIM_CCER_CC4E     (1UL << 12)

/* ---- SysTick ---- */
#define SYSTICK_BASE      0xE000E010UL

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

#define SysTick           ((SysTick_TypeDef *)SYSTICK_BASE)

#define SYSTICK_CTRL_ENABLE    (1UL << 0)
#define SYSTICK_CTRL_TICKINT   (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)
#define SYSTICK_CTRL_COUNTFLAG (1UL << 16)

/* ---- NVIC ---- */
#define NVIC_ISER_BASE    0xE000E100UL
#define NVIC_ICER_BASE    0xE000E180UL
#define NVIC_ISPR_BASE    0xE000E200UL
#define NVIC_IPR_BASE     0xE000E400UL

typedef struct {
    volatile uint32_t ISER[8];
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];
} NVIC_TypeDef;

#define NVIC              ((NVIC_TypeDef *)NVIC_ISER_BASE)

/* IRQ numbers for STM32F103 */
#define TIM2_IRQn         28
#define TIM3_IRQn         29
#define USART1_IRQn       37
#define ADC1_2_IRQn       18

static inline void NVIC_EnableIRQ(uint32_t irqn) {
    NVIC->ISER[irqn >> 5] = (1UL << (irqn & 0x1F));
}

static inline void NVIC_DisableIRQ(uint32_t irqn) {
    NVIC->ICER[irqn >> 5] = (1UL << (irqn & 0x1F));
}

static inline void NVIC_SetPriority(uint32_t irqn, uint8_t priority) {
    NVIC->IP[irqn] = (priority << 4) & 0xF0;
}

/* ---- SCB (for system reset) ---- */
#define SCB_AIRCR         (*(volatile uint32_t *)0xE000ED0CUL)
#define SCB_AIRCR_VECTKEY (0x05FAUL << 16)
#define SCB_AIRCR_SYSRESET (1UL << 2)

/* ---- IWDG (Independent Watchdog) ---- */
#define IWDG_BASE         0x40003000UL

typedef struct {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
} IWDG_TypeDef;

#define IWDG              ((IWDG_TypeDef *)IWDG_BASE)

#define IWDG_KEY_RELOAD   0xAAAAUL
#define IWDG_KEY_ENABLE   0xCCCCUL
#define IWDG_KEY_ACCESS   0x5555UL

/* ---- Compiler intrinsics ---- */
static inline void __disable_irq(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void __enable_irq(void)  { __asm volatile ("cpsie i" ::: "memory"); }
static inline void __WFI(void)         { __asm volatile ("wfi"); }
static inline void __DSB(void)         { __asm volatile ("dsb 0xF" ::: "memory"); }
static inline void __ISB(void)         { __asm volatile ("isb 0xF" ::: "memory"); }
static inline void __NOP(void)         { __asm volatile ("nop"); }

#endif /* STM32F103_H */
