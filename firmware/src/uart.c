/**
 * UART1 Driver — PA9 (TX), PA10 (RX)
 * Interrupt-driven receive with ring buffer.
 */

#include "uart.h"
#include "gpio.h"

static volatile char    rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

void uart_init(uint32_t baudrate) {
    /* Enable USART1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9  = USART1_TX  (AF push-pull, 50 MHz) */
    gpio_set_mode(GPIOA, 9, PIN_MODE_AF_PP_50MHZ);

    /* PA10 = USART1_RX  (input floating) */
    gpio_set_mode(GPIOA, 10, PIN_MODE_INPUT_FLOAT);

    /* Baudrate: USART1 is on APB2 = 72 MHz */
    USART1->BRR = 72000000UL / baudrate;

    /* Enable TX, RX, RXNE interrupt, USART */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

    /* Enable USART1 interrupt in NVIC */
    NVIC_SetPriority(USART1_IRQn, 3);
    NVIC_EnableIRQ(USART1_IRQn);
}

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        char c = (char)(USART1->DR & 0xFF);
        uint16_t next = (rx_head + 1) % UART_RX_BUF_SIZE;
        if (next != rx_tail) {
            rx_buf[rx_head] = c;
            rx_head = next;
        }
    }
    /* Clear overrun if set (read SR then DR) */
    if (USART1->SR & USART_SR_ORE) {
        (void)USART1->DR;
    }
}

void uart_putc(char c) {
    while (!(USART1->SR & USART_SR_TXE))
        ;
    USART1->DR = (uint32_t)c;
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

bool uart_rx_available(void) {
    return rx_head != rx_tail;
}

char uart_getc(void) {
    while (!uart_rx_available())
        ;
    char c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;
    return c;
}

uint16_t uart_read_line(char *buf, uint16_t max_len) {
    uint16_t i = 0;
    while (i < max_len - 1) {
        if (!uart_rx_available())
            break;
        char c = uart_getc();
        if (c == '\r' || c == '\n') {
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

void uart_print_int(int32_t val) {
    char buf[12];
    int i = 0;
    uint32_t uval;

    if (val < 0) {
        uart_putc('-');
        uval = (uint32_t)(-(val + 1)) + 1;
    } else {
        uval = (uint32_t)val;
    }

    do {
        buf[i++] = '0' + (uval % 10);
        uval /= 10;
    } while (uval > 0);

    while (i > 0)
        uart_putc(buf[--i]);
}

void uart_print_hex(uint32_t val) {
    static const char hex[] = "0123456789ABCDEF";
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}
