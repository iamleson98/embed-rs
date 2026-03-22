# STM32F103 Thermal Monitor - C++ Implementation

Modern C++17 implementation of the bare-metal thermal monitoring firmware for STM32F103C8T6 (Blue Pill).

## Features

This implementation demonstrates **modern C++ best practices** for embedded systems:

### Object-Oriented Design
- **RAII Principles**: Hardware resources managed through constructors/destructors
- **Type-Safe Wrappers**: Strong typing for GPIO pins, peripherals, and states
- **Encapsulation**: Hardware details hidden behind clean interfaces

### Modern C++ Features
- **C++17 Standard**: `std::string_view`, `std::optional`, `std::array`
- **Compile-Time Configuration**: `constexpr` for zero-overhead configuration
- **Templates**: Generic, reusable components
- **No Exceptions/RTTI**: Optimized for embedded systems (`-fno-exceptions -fno-rtti`)

### Architecture Highlights
- **Class-Based Peripherals**: Each peripheral (GPIO, UART, ADC) wrapped in type-safe classes
- **State Machine**: Enumerated states with compile-time string mapping
- **Composition**: `ThermalMonitor` composes sensors, controllers, and watchdog
- **Separation of Concerns**: Hardware abstraction, business logic, and application clearly separated

## Project Structure

```
firmware-cpp/
├── CMakeLists.txt          # Modern CMake build system
├── linker.ld               # Linker script (64K Flash, 20K SRAM)
├── include/
│   ├── stm32f103.hpp       # Type-safe register definitions
│   ├── system.hpp          # Clock & timing (namespace-based)
│   ├── gpio.hpp            # GPIO Pin class (RAII)
│   ├── uart.hpp            # UART class with std::string_view
│   ├── adc.hpp             # ADC namespace API
│   └── timer.hpp           # Timer/PWM namespace API
└── src/
    ├── main.cpp            # Application (OOP state machine)
    ├── system.cpp          # System initialization
    ├── gpio.cpp            # GPIO implementation
    ├── uart.cpp            # UART implementation
    ├── adc.cpp             # ADC implementation
    ├── timer.cpp           # Timer implementation
    └── startup.c           # Vector table (C)
```

## Building

### Prerequisites

```bash
# ARM GCC toolchain
sudo apt install gcc-arm-none-eabi g++-arm-none-eabi cmake

# Or on macOS
brew install arm-none-eabi-gcc cmake
```

### Compile with CMake

```bash
cd firmware-cpp
mkdir build && cd build
cmake ..
make
```

Output files:
- `build/thermal_monitor.elf` - Debuggable ELF
- `build/thermal_monitor.bin` - Raw binary
- `build/thermal_monitor.hex` - Intel HEX
- `build/thermal_monitor.map` - Memory map

### Flash

```bash
make flash
```

## Code Examples

### Type-Safe GPIO

```cpp
gpio::Pin led(GPIOC, 13);
led.set_mode(gpio::Mode::Output_2MHz, gpio::Config::OutputPushPull);
led.write(gpio::State::High);
led.toggle();
```

### UART with std::string_view

```cpp
auto& uart = uart::instance();
uart.put_string("Temperature: ");
uart.print_int(temp_mc / 1000);
uart.put_char('\n');
```

### Object Composition

```cpp
class ThermalMonitor {
    gpio::Pin status_led_;
    TemperatureSensor temp_ext_;
    FanController fan_;
    Watchdog watchdog_;
    // ...
};
```

### Compile-Time Configuration

```cpp
namespace config {
    constexpr int32_t TEMP_THRESHOLD_LOW_MC = 30000;
    constexpr uint32_t FAN_PWM_FREQ_HZ = 25000;
}
```

## Differences from C Version

| Aspect | C Version | C++ Version |
|--------|-----------|-------------|
| **Peripheral Access** | Direct register manipulation | Type-safe wrapper classes |
| **State Management** | Global variables + enums | Class members + strong enums |
| **GPIO** | Function calls with port/pin | `gpio::Pin` RAII object |
| **Configuration** | `#define` macros | `constexpr` variables |
| **String Handling** | C strings | `std::string_view` |
| **Optional Values** | Sentinel values | `std::optional<T>` |
| **Arrays** | C arrays | `std::array<T, N>` |

## Key Classes

### `ThermalMonitor`
Main application orchestrating sensors, fan control, and state machine

### `TemperatureSensor`
IIR-filtered temperature reading with encapsulated state

### `FanController`
Automatic/manual fan control with duty cycle computation

### `Watchdog`
RAII watchdog timer (initialized on construction)

### `gpio::Pin`
Type-safe GPIO pin with move semantics (no copies)

## Benefits of C++ Approach

1. **Type Safety**: Compile-time error detection (wrong pin types, invalid states)
2. **Code Reuse**: Generic components easily extended
3. **Maintainability**: Clear ownership and lifecycle management
4. **Modern Tooling**: Better IDE support, refactoring tools
5. **Zero Overhead**: C++ abstractions compile to identical assembly as C
6. **Testability**: Mock peripherals for unit testing

## Technical Details

- **Clock**: HSE 8 MHz → PLL ×9 → 72 MHz
- **Compiler Flags**: `-Os -fno-exceptions -fno-rtti -ffunction-sections`
- **Standard Library**: Minimal usage (no heap, no exceptions)
- **Code Size**: Comparable to C version (~8-10 KB)

## License

Public domain — use however you like.
