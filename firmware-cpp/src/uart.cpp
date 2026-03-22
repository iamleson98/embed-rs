/**
 * UART Driver Implementation (C++)
 */

#include "uart.hpp"
#include "stm32f103.hpp"

namespace uart {

static UART* g_instance = nullptr;

void init(uint32_t baudrate) {
    g_instance = new UART(baudrate);
}

UART& instance() {
    return *g_instance;
}

UART::UART(uint32_t baudrate) {
    using namespace stm32;

    // Enable USART1 clock
    RCC->APB2ENR |= rcc::APB2ENR_USART1EN | rcc::APB2ENR_IOPAEN;

    // Configure PA9 (TX) as alternate function push-pull
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);  // 50MHz, AF push-pull

    // Configure PA10 (RX) as input floating
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |= (0x4 << 8);

    // Configure USART1: 8N1
    USART1->BRR = 72000000 / baudrate;  // 72 MHz / baudrate
    USART1->CR1 = usart::CR1_UE | usart::CR1_TE | usart::CR1_RE | (1 << 5); // RXNEIE

    // Enable USART1 interrupt
    *reinterpret_cast<volatile uint32_t*>(0xE000E100) |= (1 << 37); // NVIC_ISER1
}

void UART::put_char(char c) {
    while (!(stm32::USART1->SR & stm32::usart::SR_TXE)) {}
    stm32::USART1->DR = c;
}

void UART::put_string(std::string_view str) {
    for (char c : str) {
        put_char(c);
    }
}

void UART::print_int(int32_t val) {
    if (val < 0) {
        put_char('-');
        val = -val;
    }

    char buf[12];
    int i = 0;

    if (val == 0) {
        put_char('0');
        return;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i > 0) {
        put_char(buf[--i]);
    }
}

std::optional<char> UART::get_char() {
    if (rx_head_ == rx_tail_) {
        return std::nullopt;
    }

    char c = rx_buffer_[rx_tail_];
    rx_tail_ = (rx_tail_ + 1) % RX_BUF_SIZE;
    return c;
}

bool UART::rx_available() const {
    return rx_head_ != rx_tail_;
}

uint16_t UART::read_line(char* buffer, uint16_t max_len) {
    uint16_t idx = 0;

    while (idx < max_len - 1) {
        auto c = get_char();
        if (!c) break;

        if (*c == '\n' || *c == '\r') {
            if (idx > 0) break;
            continue;
        }

        buffer[idx++] = *c;
    }

    buffer[idx] = '\0';
    return idx;
}

} // namespace uart

extern "C" void USART1_IRQHandler() {
    if (stm32::USART1->SR & stm32::usart::SR_RXNE) {
        auto& uart_inst = uart::instance();
        char c = static_cast<char>(stm32::USART1->DR);

        uint16_t next_head = (uart_inst.rx_head_ + 1) % uart::UART::RX_BUF_SIZE;
        if (next_head != uart_inst.rx_tail_) {
            uart_inst.rx_buffer_[uart_inst.rx_head_] = c;
            uart_inst.rx_head_ = next_head;
        }
    }
}
