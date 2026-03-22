/**
 * UART Driver Interface
 */

#ifndef UART_H
#define UART_H

#include "stm32f103.h"
#include <stdint.h>
#include <stdbool.h>

#define UART_RX_BUF_SIZE 64

void uart_init(uint32_t baudrate);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_print_int(int32_t val);
void uart_print_hex(uint32_t val);
bool uart_rx_available(void);
char uart_getc(void);
uint16_t uart_read_line(char *buf, uint16_t max_len);

#endif /* UART_H */
