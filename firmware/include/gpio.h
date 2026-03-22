/**
 * GPIO Driver Interface
 */

#ifndef GPIO_H
#define GPIO_H

#include "stm32f103.h"

typedef enum {
    PIN_MODE_INPUT_ANALOG = 0,
    PIN_MODE_INPUT_FLOAT,
    PIN_MODE_INPUT_PUPD,
    PIN_MODE_OUTPUT_PP_2MHZ,
    PIN_MODE_OUTPUT_PP_10MHZ,
    PIN_MODE_OUTPUT_PP_50MHZ,
    PIN_MODE_AF_PP_50MHZ,
    PIN_MODE_AF_PP_10MHZ,
} gpio_mode_t;

void gpio_init(void);
void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin);
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_H */
