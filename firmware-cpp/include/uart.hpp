/**
 * UART Driver (C++)
 * Type-safe UART communication
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <optional>

namespace uart {

/**
 * UART peripheral wrapper
 */
class UART {
public:
    explicit UART(uint32_t baudrate);

    void put_char(char c);
    void put_string(std::string_view str);
    void print_int(int32_t val);

    std::optional<char> get_char();
    bool rx_available() const;
    uint16_t read_line(char* buffer, uint16_t max_len);

    // Delete copy, allow move
    UART(const UART&) = delete;
    UART& operator=(const UART&) = delete;
    UART(UART&&) = default;
    UART& operator=(UART&&) = default;

private:
    static constexpr uint16_t RX_BUF_SIZE = 256;

    char rx_buffer_[RX_BUF_SIZE];
    volatile uint16_t rx_head_{0};
    volatile uint16_t rx_tail_{0};

    friend void ::USART1_IRQHandler();
};

// Global UART instance management
void init(uint32_t baudrate);
UART& instance();

} // namespace uart
