# STM32F103 Thermal Monitor - Rust Implementation

Modern **Rust 2021** embedded firmware for STM32F103C8T6 demonstrating idiomatic Rust patterns and best practices for embedded systems.

## Features

This implementation showcases **modern Rust embedded development**:

### Rust-Specific Features
- **Type Safety**: Compile-time guarantees for hardware access
- **Ownership System**: Zero-cost resource management without garbage collection
- **Trait-Based HAL**: Generic interfaces for peripheral abstraction
- **Zero-Cost Abstractions**: High-level code compiles to efficient machine code
- **Memory Safety**: No undefined behavior, no null pointers, no dangling references
- **Fearless Concurrency**: Safe interrupt handling with `Mutex` and `Cell`

### Embedded Rust Ecosystem
- **`cortex-m`**: Cortex-M processor support
- **`cortex-m-rt`**: Minimal startup runtime
- **`stm32f1xx-hal`**: Type-safe STM32F1 HAL with `embedded-hal` traits
- **`embedded-hal`**: Hardware abstraction layer traits
- **No Standard Library** (`#![no_std]`): Optimized for constrained environments

### Design Patterns
- **Newtype Pattern**: Strong typing for hardware resources
- **Builder Pattern**: Fluent clock configuration
- **RAII**: Automatic resource management through Drop trait
- **Type State Pattern**: Compile-time pin configuration
- **Interior Mutability**: Safe shared mutable state in interrupts

## Project Structure

```
firmware-rust/
├── Cargo.toml              # Dependencies and build config
├── memory.x                # Memory layout (linker script)
├── .cargo/
│   └── config.toml         # Target and build configuration
├── src/
│   └── main.rs             # Application with HAL usage
└── build.rs                # Build-time configuration (optional)
```

## Building

### Prerequisites

Install Rust and ARM target:

```bash
# Install Rust (if not already installed)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Add ARM Cortex-M target
rustup target add thumbv7m-none-eabi

# Install cargo-binutils for flashing
cargo install cargo-binutils
rustup component add llvm-tools-preview

# Optional: Install probe-rs for debugging
cargo install probe-rs --features cli
```

### Compile

```bash
cd firmware-rust
cargo build --release
```

Output:
- `target/thumbv7m-none-eabi/release/thermal-monitor-rust` - ELF binary

### Generate Binary and Hex

```bash
# Binary format
cargo objcopy --release -- -O binary thermal-monitor.bin

# Intel HEX format
cargo objcopy --release -- -O ihex thermal-monitor.hex
```

### Flash

#### Using OpenOCD
```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program target/thumbv7m-none-eabi/release/thermal-monitor-rust verify reset exit"
```

#### Using probe-rs
```bash
probe-rs run --chip STM32F103C8 target/thumbv7m-none-eabi/release/thermal-monitor-rust
```

### Debug

```bash
# Launch OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# In another terminal
cargo run --release
```

## Code Examples

### Type-Safe Clock Configuration

```rust
let clocks = rcc
    .cfgr
    .use_hse(8.MHz())
    .sysclk(72.MHz())
    .pclk1(36.MHz())
    .freeze(&mut flash.acr);
```

### Type State Pattern for GPIO

```rust
// Pin starts in unknown state
let pin = gpioc.pc13;

// Convert to output (type changes!)
let mut led = pin.into_push_pull_output(&mut gpioc.crh);

// Safe operations
led.set_high();
led.toggle();
```

### Trait-Based PWM

```rust
impl<P: embedded_hal::pwm::SetDutyCycle> FanController<P> {
    fn new(pwm_channel: P) -> Self { /* ... */ }
}

// Works with any PWM implementation
let pwm = timer.pwm_hz(pin, 25.kHz(), &clocks);
let mut fan = FanController::new(pwm);
```

### Safe Interrupt Handling

```rust
static G_ADC_SAMPLE: Mutex<Cell<Option<u16>>> = Mutex::new(Cell::new(None));

// In ISR
free(|cs| {
    G_ADC_SAMPLE.borrow(cs).set(Some(value));
});

// In main loop
if let Some(sample) = free(|cs| G_ADC_SAMPLE.borrow(cs).take()) {
    // Process sample
}
```

### Ownership-Based Resource Management

```rust
// Serial owns TX and RX pins
let serial = Serial::new(
    usart1,
    (tx_pin, rx_pin),  // Ownership transferred
    config,
    &clocks,
);

// Pins cannot be used elsewhere
```

## Rust Benefits for Embedded

| Feature | Benefit |
|---------|---------|
| **No Null Pointers** | Use `Option<T>` instead |
| **No Data Races** | Enforced by borrow checker |
| **No Buffer Overflows** | Array bounds checked |
| **No Use-After-Free** | Prevented by ownership |
| **No Uninitialized Memory** | All variables must be initialized |
| **Type Safety** | Wrong peripheral usage caught at compile time |

## Differences from C/C++

| Aspect | C/C++ | Rust |
|--------|-------|------|
| **Memory Safety** | Manual, error-prone | Automatic, compile-time verified |
| **Peripheral Access** | Volatile pointers | PAC (Peripheral Access Crate) |
| **HAL** | Function calls | Zero-cost trait abstractions |
| **Interrupts** | Raw ISRs with globals | Safe with `Mutex<Cell<T>>` |
| **Configuration** | Macros/constexpr | `const` with full type system |
| **Error Handling** | Return codes | `Result<T, E>` with `?` operator |
| **Generics** | Templates (monomorphization) | Zero-cost trait objects |

## Key Rust Constructs

### `#![no_std]`
No standard library - uses `core` (subset for embedded)

### `#[entry]`
Marks program entry point (replaces `main()` from startup code)

### `#[interrupt]`
Marks interrupt handler function

### `Mutex<RefCell<T>>`
Safe shared mutable state for interrupts

### `Cell<T>`
Interior mutability for `Copy` types

### Traits
- `embedded_hal::digital::OutputPin`
- `embedded_hal::pwm::SetDutyCycle`
- `embedded_hal::serial::Write`

## Memory Usage

```bash
cargo size --release -- -A
```

Typical sizes:
- `.text` (code): ~6-8 KB
- `.rodata` (constants): ~1 KB
- `.data` (initialized): ~100 bytes
- `.bss` (zero-initialized): ~500 bytes

## Optimization Levels

Configured in `Cargo.toml`:

```toml
[profile.release]
opt-level = "z"       # Optimize for size
lto = true            # Link-time optimization
codegen-units = 1     # Better optimization
```

## Testing

```bash
# Check compilation
cargo check

# Build with all warnings
cargo build --release

# Clippy lints
cargo clippy

# Format code
cargo fmt
```

## License

Public domain — use however you like.
