/**
 * GPIO Driver Implementation (C++)
 */

#include "gpio.hpp"

namespace gpio {

void init() {
    using namespace stm32;
    // Enable GPIO clocks
    RCC->APB2ENR |= rcc::APB2ENR_IOPAEN | rcc::APB2ENR_IOPCEN;
}

Pin::Pin(stm32::GPIO_Regs* port, uint8_t pin_num)
    : port_(port), pin_num_(pin_num) {}

void Pin::set_mode(Mode mode, Config config) {
    volatile uint32_t* config_reg = (pin_num_ < 8) ? &port_->CRL : &port_->CRH;
    uint8_t pos = (pin_num_ % 8) * 4;

    uint32_t temp = *config_reg;
    temp &= ~(0xF << pos);  // Clear bits
    temp |= (static_cast<uint32_t>(config) << (pos + 2)) |
            (static_cast<uint32_t>(mode) << pos);
    *config_reg = temp;
}

void Pin::write(State state) {
    if (state == State::High) {
        port_->BSRR = (1 << pin_num_);
    } else {
        port_->BSRR = (1 << (pin_num_ + 16));
    }
}

State Pin::read() const {
    return (port_->IDR & (1 << pin_num_)) ? State::High : State::Low;
}

void Pin::toggle() {
    if (read() == State::High) {
        write(State::Low);
    } else {
        write(State::High);
    }
}

} // namespace gpio
