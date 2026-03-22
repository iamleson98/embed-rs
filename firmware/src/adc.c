/**
 * ADC1 Driver
 * Supports single-channel conversion + internal temperature sensor (ch16).
 * ADC clock = 12 MHz (72 MHz / 6).
 */

#include "adc.h"
#include "stm32f103.h"
#include "gpio.h"
#include "system.h"

void adc_init(void) {
    /* Enable ADC1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* PA0 = ADC channel 0 (analog input for external sensor) */
    gpio_set_mode(GPIOA, 0, PIN_MODE_INPUT_ANALOG);

    /* Reset ADC configuration */
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    /* Enable internal temp sensor + Vrefint */
    ADC1->CR2 |= ADC_CR2_TSVREFE;

    /* Set sample time for ch0 = 55.5 cycles, ch16 (temp) = 239.5 cycles */
    ADC1->SMPR2 = (0x5UL << (0 * 3));   /* ch0:  55.5 cycles  */
    ADC1->SMPR1 = (0x7UL << (6 * 3));   /* ch16: 239.5 cycles (16-10=6 offset in SMPR1) */

    /* Use SWSTART as external trigger */
    ADC1->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;

    /* Power on ADC */
    ADC1->CR2 |= ADC_CR2_ADON;
    delay_ms(2);  /* ADC stabilization time */

    /* Calibration */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL)
        ;
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL)
        ;
}

uint16_t adc_read(uint8_t channel) {
    /* Set channel in sequence register (single conversion, 1 channel) */
    ADC1->SQR1 = 0;                              /* 1 conversion  */
    ADC1->SQR3 = (uint32_t)(channel & 0x1F);     /* Channel       */

    /* Clear EOC flag */
    ADC1->SR &= ~ADC_SR_EOC;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for completion */
    while (!(ADC1->SR & ADC_SR_EOC))
        ;

    return (uint16_t)(ADC1->DR & 0xFFF);
}

/**
 * Read internal temperature sensor.
 * Formula from reference manual:
 *   T(°C) = ((V25 - Vsense) / Avg_Slope) + 25
 *   V25 = 1.43V, Avg_Slope = 4.3 mV/°C
 *   With 3.3V reference and 12-bit ADC:
 *     Vsense = raw * 3300 / 4096 (in mV)
 *
 * Returns temperature in milli-degrees Celsius for better precision.
 */
int32_t adc_read_temperature_mc(void) {
    uint16_t raw = adc_read(16);
    int32_t vsense_mv = ((int32_t)raw * 3300) / 4096;
    int32_t temp_mc = ((1430 - vsense_mv) * 1000) / 43 + 25000;
    return temp_mc;
}
