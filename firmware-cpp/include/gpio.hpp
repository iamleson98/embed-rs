/**
 * GPIO Driver (C++)
 * Type-safe GPIO abstraction using modern C++
 */

#pragma once

#include "stm32f103.hpp"
#include <cstdint>

namespace gpio {

enum class Mode : uint8_t {
    Input = 0,
    Output_10MHz = 1,
    Output_2MHz = 2,
    Output_50MHz = 3
};

enum class Config : uint8_t {
    Analog = 0,
    InputFloating = 1,
    InputPullUpDown = 2,
    OutputPushPull = 0,
    OutputOpenDrain = 1,
    AltPushPull = 2,
    AltOpenDrain = 3
};

enum class State : bool {
    Low = false,
    High = true
};

/**
 * RAII GPIO pin wrapper
 */
class Pin {
public:
    Pin(stm32::GPIO_Regs* port, uint8_t pin_num);

    void set_mode(Mode mode, Config config);
    void write(State state);
    State read() const;
    void toggle();

    // Delete copy, allow move
    Pin(const Pin&) = delete;
    Pin& operator=(const Pin&) = delete;
    Pin(Pin&&) = default;
    Pin& operator=(Pin&&) = default;

private:
    stm32::GPIO_Regs* port_;
    uint8_t pin_num_;
};

/**
 * Initialize GPIO system
 */
void init();

} // namespace gpio
