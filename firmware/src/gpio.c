/**
 * GPIO Driver
 */

#include "gpio.h"

void gpio_init(void) {
    /* Enable GPIO port clocks: A, B, C + AFIO */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
                   | RCC_APB2ENR_IOPBEN
                   | RCC_APB2ENR_IOPCEN
                   | RCC_APB2ENR_AFIOEN;
}

void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode) {
    uint32_t config;

    switch (mode) {
    case PIN_MODE_INPUT_ANALOG:
        config = GPIO_MODE_INPUT | GPIO_CNF_IN_ANALOG;
        break;
    case PIN_MODE_INPUT_FLOAT:
        config = GPIO_MODE_INPUT | GPIO_CNF_IN_FLOAT;
        break;
    case PIN_MODE_INPUT_PUPD:
        config = GPIO_MODE_INPUT | GPIO_CNF_IN_PUPD;
        break;
    case PIN_MODE_OUTPUT_PP_2MHZ:
        config = GPIO_MODE_OUT_2MHZ | GPIO_CNF_OUT_PP;
        break;
    case PIN_MODE_OUTPUT_PP_10MHZ:
        config = GPIO_MODE_OUT_10MHZ | GPIO_CNF_OUT_PP;
        break;
    case PIN_MODE_OUTPUT_PP_50MHZ:
        config = GPIO_MODE_OUT_50MHZ | GPIO_CNF_OUT_PP;
        break;
    case PIN_MODE_AF_PP_50MHZ:
        config = GPIO_MODE_OUT_50MHZ | GPIO_CNF_AF_PP;
        break;
    case PIN_MODE_AF_PP_10MHZ:
        config = GPIO_MODE_OUT_10MHZ | GPIO_CNF_AF_PP;
        break;
    default:
        return;
    }

    volatile uint32_t *cr = (pin < 8) ? &port->CRL : &port->CRH;
    uint8_t shift = (pin % 8) * 4;

    *cr = (*cr & ~(0xFUL << shift)) | (config << shift);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value) {
    if (value) {
        port->BSRR = (1UL << pin);         /* Set   */
    } else {
        port->BSRR = (1UL << (pin + 16));  /* Reset */
    }
}

void gpio_toggle(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR ^= (1UL << pin);
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin) {
    return (port->IDR >> pin) & 1U;
}
