/**
 * STM32F103 Startup Code
 * Vector table + Reset handler (data/bss init)
 */

#include <stdint.h>

/* Linker symbols */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/* Main entry point */
extern int main(void);

/* System init (clock config) */
extern void SystemInit(void);

/* Default handler for unused interrupts */
void Default_Handler(void) {
    while (1) { __asm volatile ("bkpt #0"); }
}

/* Cortex-M3 exception handlers (weak, can be overridden) */
void Reset_Handler(void);
void NMI_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)     __attribute__((weak, alias("Default_Handler")));

/* STM32F103 peripheral interrupt handlers (weak) */
void WWDG_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void USBWakeUp_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));

/* Vector table — placed at 0x08000000 by linker */
__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    &_estack,                          /* Initial stack pointer */
    (void *)Reset_Handler,             /* Reset */
    (void *)NMI_Handler,               /* NMI */
    (void *)HardFault_Handler,         /* Hard Fault */
    (void *)MemManage_Handler,         /* MPU Fault */
    (void *)BusFault_Handler,          /* Bus Fault */
    (void *)UsageFault_Handler,        /* Usage Fault */
    0, 0, 0, 0,                        /* Reserved */
    (void *)SVC_Handler,               /* SVCall */
    (void *)DebugMon_Handler,          /* Debug Monitor */
    0,                                 /* Reserved */
    (void *)PendSV_Handler,            /* PendSV */
    (void *)SysTick_Handler,           /* SysTick */

    /* STM32F103 peripheral interrupts */
    (void *)WWDG_IRQHandler,           /* 0  */
    (void *)PVD_IRQHandler,            /* 1  */
    (void *)TAMPER_IRQHandler,         /* 2  */
    (void *)RTC_IRQHandler,            /* 3  */
    (void *)FLASH_IRQHandler,          /* 4  */
    (void *)RCC_IRQHandler,            /* 5  */
    (void *)EXTI0_IRQHandler,          /* 6  */
    (void *)EXTI1_IRQHandler,          /* 7  */
    (void *)EXTI2_IRQHandler,          /* 8  */
    (void *)EXTI3_IRQHandler,          /* 9  */
    (void *)EXTI4_IRQHandler,          /* 10 */
    (void *)DMA1_Channel1_IRQHandler,  /* 11 */
    (void *)DMA1_Channel2_IRQHandler,  /* 12 */
    (void *)DMA1_Channel3_IRQHandler,  /* 13 */
    (void *)DMA1_Channel4_IRQHandler,  /* 14 */
    (void *)DMA1_Channel5_IRQHandler,  /* 15 */
    (void *)DMA1_Channel6_IRQHandler,  /* 16 */
    (void *)DMA1_Channel7_IRQHandler,  /* 17 */
    (void *)ADC1_2_IRQHandler,         /* 18 */
    (void *)USB_HP_CAN1_TX_IRQHandler, /* 19 */
    (void *)USB_LP_CAN1_RX0_IRQHandler,/* 20 */
    (void *)CAN1_RX1_IRQHandler,       /* 21 */
    (void *)CAN1_SCE_IRQHandler,       /* 22 */
    (void *)EXTI9_5_IRQHandler,        /* 23 */
    (void *)TIM1_BRK_IRQHandler,       /* 24 */
    (void *)TIM1_UP_IRQHandler,        /* 25 */
    (void *)TIM1_TRG_COM_IRQHandler,   /* 26 */
    (void *)TIM1_CC_IRQHandler,        /* 27 */
    (void *)TIM2_IRQHandler,           /* 28 */
    (void *)TIM3_IRQHandler,           /* 29 */
    (void *)TIM4_IRQHandler,           /* 30 */
    (void *)I2C1_EV_IRQHandler,        /* 31 */
    (void *)I2C1_ER_IRQHandler,        /* 32 */
    (void *)I2C2_EV_IRQHandler,        /* 33 */
    (void *)I2C2_ER_IRQHandler,        /* 34 */
    (void *)SPI1_IRQHandler,           /* 35 */
    (void *)SPI2_IRQHandler,           /* 36 */
    (void *)USART1_IRQHandler,         /* 37 */
    (void *)USART2_IRQHandler,         /* 38 */
    (void *)USART3_IRQHandler,         /* 39 */
    (void *)EXTI15_10_IRQHandler,      /* 40 */
    (void *)RTC_Alarm_IRQHandler,      /* 41 */
    (void *)USBWakeUp_IRQHandler,      /* 42 */
};

/**
 * Reset Handler — called on power-on / reset
 * Copies .data from Flash to SRAM, zeros .bss, then calls main.
 */
void __attribute__((noreturn)) Reset_Handler(void) {
    uint32_t *src, *dst;

    /* Copy .data section from Flash to SRAM */
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero .bss section */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* Configure clocks before main */
    SystemInit();

    /* Call main */
    (void)main();

    /* Should never return — trap */
    while (1) {
        __asm volatile ("bkpt #0");
    }
}
