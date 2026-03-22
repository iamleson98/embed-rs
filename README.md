# embed-rs

Embedded systems development with modern C, C++, and Rust implementations for STM32F103C8T6 (Blue Pill).

## Overview

This repository contains **three implementations** of a bare-metal thermal monitoring system, each demonstrating best practices and modern features of their respective languages:

1. **C** - Traditional bare-metal firmware (`/firmware`)
2. **C++** - Modern C++17 with OOP and RAII (`/firmware-cpp`)
3. **Rust** - Safe embedded Rust with HAL (`/firmware-rust`)

All implementations provide the same functionality but showcase different programming paradigms and language features.

## Features

All versions include:

- **Temperature Monitoring**: External NTC thermistor + internal temp sensor
- **Digital Filtering**: IIR low-pass filter for noise rejection
- **PWM Fan Control**: Automatic duty cycle based on temperature thresholds
- **UART Telemetry**: Real-time reporting at 115200 baud
- **State Machine**: NORMAL → WARNING → CRITICAL states
- **LED Status Indicators**: Blink patterns indicate system state
- **Interactive Commands**: `status`, `fan <0-100|auto>`, `reset`
- **Watchdog Timer**: System reliability with automatic reset

## Project Structure

```
embed-rs/
├── firmware/           # C implementation (bare-metal)
│   ├── include/        # Register definitions, drivers
│   ├── src/            # Implementation files
│   ├── Makefile        # GCC build system
│   └── README.md       # C-specific documentation
│
├── firmware-cpp/       # C++ implementation (modern C++17)
│   ├── include/        # Type-safe wrappers, classes
│   ├── src/            # OOP implementation
│   ├── CMakeLists.txt  # CMake build system
│   └── README.md       # C++-specific documentation
│
├── firmware-rust/      # Rust implementation (safe embedded)
│   ├── src/            # Rust application
│   ├── Cargo.toml      # Dependencies and config
│   ├── memory.x        # Memory layout
│   └── README.md       # Rust-specific documentation
│
└── src/                # Root Rust example (simple)
    └── main.rs
```

## Quick Start

### C Version
```bash
cd firmware
make
# Output: build/thermal_monitor.{elf,bin,hex}
```

### C++ Version
```bash
cd firmware-cpp
mkdir build && cd build
cmake ..
make
# Output: build/thermal_monitor.{elf,bin,hex}
```

### Rust Version
```bash
cd firmware-rust
cargo build --release
cargo objcopy --release -- -O binary thermal-monitor.bin
# Output: target/thumbv7m-none-eabi/release/thermal-monitor-rust
```

## Language Comparison

| Feature | C | C++ | Rust |
|---------|---|-----|------|
| **Type Safety** | Weak | Strong | Strongest |
| **Memory Safety** | Manual | Manual | Automatic |
| **Abstractions** | Functions | Classes/Templates | Traits/Generics |
| **Code Size** | ~8 KB | ~8-10 KB | ~6-8 KB |
| **Build System** | Makefile | CMake | Cargo |
| **HAL** | Register access | Wrapped classes | Type-safe traits |
| **Learning Curve** | Low | Medium | High |
| **Compile-Time Checks** | Minimal | Good | Excellent |

## Implementation Highlights

### C (`/firmware`)
- **Direct register manipulation** for maximum control
- **Function-based drivers** for UART, GPIO, ADC, Timer
- **Minimal abstractions** - close to hardware
- **Traditional embedded C patterns**

**Best for**: Maximum performance, minimal footprint, learning hardware

### C++ (`/firmware-cpp`)
- **Type-safe peripheral wrappers** with RAII
- **Object-oriented design** (classes for peripherals, sensors, controllers)
- **Modern C++17 features** (`constexpr`, `std::string_view`, `std::optional`)
- **Zero-overhead abstractions** compiled to same code as C

**Best for**: Maintainable code, type safety, reusable components

### Rust (`/firmware-rust`)
- **Memory safety guarantees** at compile time
- **Ownership-based resource management** (no leaks, no data races)
- **HAL with trait abstractions** (`embedded-hal`)
- **Safe concurrency** for interrupt handling

**Best for**: Safety-critical systems, concurrent code, preventing bugs

## Hardware Setup

### Pin Mapping

| Pin  | Function              |
|------|-----------------------|
| PC13 | Status LED (active low) |
| PA0  | ADC CH0 (NTC thermistor) |
| PA6  | TIM3 CH1 (Fan PWM) |
| PA9  | USART1 TX |
| PA10 | USART1 RX |

### Temperature Thresholds

- **< 30°C**: Fan off (NORMAL state)
- **30-45°C**: Fan ramps 0→50% (NORMAL→WARNING)
- **45-60°C**: Fan ramps 50→100% (WARNING state)
- **60-80°C**: Fan 100% (WARNING state)
- **≥ 80°C**: Fan 100% (CRITICAL state)

### LED Blink Patterns

- **500ms**: Normal operation
- **200ms**: Warning (temp ≥ 45°C)
- **50ms**: Critical (temp ≥ 80°C)

## Flashing

All versions can be flashed using OpenOCD:

```bash
# Replace <firmware.bin> with the appropriate binary
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program <firmware.bin> 0x08000000 verify reset exit"
```

Or use the provided make/cmake targets:
- C: `make flash`
- C++: `make flash` (from build directory)
- Rust: See `firmware-rust/README.md` for probe-rs options

## Testing

### Simulation

All versions can be tested with:
- **Wokwi** (web-based STM32 simulator)
- **QEMU** (with ARM Cortex-M3 emulation)
- **Renode** (full system simulation)

See individual README files for simulation instructions.

### UART Commands

Connect to UART at **115200 baud**:

```
status              # Print system status
fan 75              # Set fan to 75% (manual override)
fan auto            # Return to automatic fan control
reset               # Software reset
```

### Expected Output

```
========================================
  STM32F103 Thermal Monitor v1.0
  Bare-metal | 72 MHz | UART 115200
========================================
Commands: status, fan <0-100|auto>, reset

[TMON] t=1s ext=28.3C int=32.1C fan=0% state=NORMAL
[TMON] t=2s ext=28.5C int=32.2C fan=0% state=NORMAL
...
```

## Prerequisites

### All Versions
- ARM GCC toolchain: `arm-none-eabi-gcc`
- OpenOCD or ST-Link tools for flashing

### C Version
```bash
sudo apt install gcc-arm-none-eabi make
```

### C++ Version
```bash
sudo apt install gcc-arm-none-eabi g++-arm-none-eabi cmake make
```

### Rust Version
```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Add ARM target
rustup target add thumbv7m-none-eabi

# Install tools
cargo install cargo-binutils
rustup component add llvm-tools-preview
```

## Documentation

Each implementation has detailed documentation:

- [C Implementation (`/firmware/README.md`)](./firmware/README.md)
- [C++ Implementation (`/firmware-cpp/README.md`)](./firmware-cpp/README.md)
- [Rust Implementation (`/firmware-rust/README.md`)](./firmware-rust/README.md)

## Learning Path

1. **Start with C** (`/firmware`) to understand hardware fundamentals
2. **Move to C++** (`/firmware-cpp`) to learn abstraction patterns
3. **Explore Rust** (`/firmware-rust`) for safety and modern features

Each version builds on concepts from the previous, demonstrating how different languages handle embedded systems.

## Technical Specifications

- **MCU**: STM32F103C8T6 (ARM Cortex-M3)
- **Flash**: 64 KB
- **RAM**: 20 KB
- **Clock**: 72 MHz (HSE 8 MHz + PLL ×9)
- **ADC**: 12-bit, dual-channel (external + internal)
- **PWM**: 25 kHz on TIM3 CH1
- **Sampling Rate**: 10 Hz (TIM2 interrupt)
- **UART**: 115200 baud, 8N1

## Contributing

Contributions are welcome! Each implementation should:
- Follow language-specific best practices
- Maintain feature parity with other versions
- Include clear documentation
- Keep code size reasonable (<10 KB per implementation)

## License

Public domain — use however you like. All code is provided as-is for educational and commercial use.

## References

- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190.pdf)
- [ARM Cortex-M3 Technical Reference](https://developer.arm.com/documentation/ddi0337/e/)
- [Embedded Rust Book](https://docs.rust-embedded.org/book/)
- [Modern C++ for Embedded](https://www.modernescpp.com/)
