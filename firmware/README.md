# STM32F103 Bare-Metal Thermal Monitor

A serious, production-style bare-metal firmware for the **STM32F103C8T6** (Blue Pill) — no HAL, no Arduino, no RTOS. All peripherals driven through direct register access.

## What It Does

- **Reads temperature** from an external NTC thermistor (ADC ch0) and the STM32 internal temperature sensor (ch16)
- **Digital IIR low-pass filter** on all ADC readings for noise rejection
- **PWM fan control** (TIM3 CH1 at 25 kHz) with automatic duty cycle based on temperature thresholds
- **UART telemetry** at 115200 baud — periodic reports every 1 second
- **State machine**: NORMAL → WARNING → CRITICAL with LED blink patterns
- **Watchdog timer** (IWDG) for reliability — 2 second timeout
- **Interactive UART commands**: `status`, `fan <0-100>`, `fan auto`, `reset`

## Project Structure

```
firmware/
├── Makefile               # Build system (arm-none-eabi-gcc)
├── linker.ld              # Linker script (64K Flash, 20K SRAM)
├── include/
│   ├── stm32f103.h        # Register definitions (CMSIS-style)
│   ├── system.h           # Clock + SysTick
│   ├── gpio.h             # GPIO driver
│   ├── uart.h             # UART driver
│   ├── adc.h              # ADC driver
│   └── timer.h            # Timer/PWM driver
├── src/
│   ├── startup.c          # Vector table + Reset_Handler
│   ├── system.c           # Clock config (HSE+PLL → 72 MHz)
│   ├── gpio.c             # GPIO pin configuration
│   ├── uart.c             # USART1 with IRQ ring buffer
│   ├── adc.c              # ADC1 single-channel + temp sensor
│   ├── timer.c            # TIM3 PWM + TIM2 sampling interrupt
│   └── main.c             # Application logic + state machine
└── wokwi/
    ├── diagram.json        # Wokwi circuit diagram
    └── wokwi.toml          # Wokwi project config
```

## Pin Map

| Pin  | Function              |
|------|-----------------------|
| PC13 | Status LED (active low) |
| PA0  | ADC ch0 — NTC thermistor |
| PA6  | TIM3 CH1 — Fan PWM output |
| PA9  | USART1 TX             |
| PA10 | USART1 RX             |

## Building

### Prerequisites

Install the ARM GCC toolchain:

```bash
# Ubuntu/Debian
sudo apt install gcc-arm-none-eabi

# macOS (Homebrew)
brew install arm-none-eabi-gcc

# Windows — download from:
# https://developer.arm.com/downloads/-/gnu-rm
```

### Compile

```bash
cd firmware
make
```

Output:
```
build/thermal_monitor.elf   # Debuggable ELF
build/thermal_monitor.bin   # Raw binary for flashing
build/thermal_monitor.hex   # Intel HEX for flashing
build/thermal_monitor.map   # Linker map (memory usage)
```

### Flash (real hardware via ST-Link)

```bash
make flash
```

Requires OpenOCD installed with an ST-Link V2 programmer.

## Simulation

### Option 1: Wokwi (Web — Recommended)

1. Go to [wokwi.com](https://wokwi.com) and create a new STM32 project
2. Upload the files from `wokwi/diagram.json` and `wokwi/wokwi.toml`
3. Build the project locally with `make` and upload `build/thermal_monitor.elf`
4. Or use the [Wokwi VS Code extension](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode) to simulate directly

### Option 2: QEMU

```bash
qemu-system-arm \
  -machine stm32vldiscovery \
  -cpu cortex-m3 \
  -nographic \
  -serial stdio \
  -kernel build/thermal_monitor.elf
```

### Option 3: Renode

```bash
renode -e "mach create; machine LoadPlatformDescription @platforms/boards/stm32f103.repl; sysbus LoadELF @build/thermal_monitor.elf; start"
```

## UART Commands

Connect at **115200 8N1**. Available commands:

| Command       | Description                          |
|---------------|--------------------------------------|
| `status`      | Print full system status             |
| `fan 75`      | Set fan to 75% (manual override)     |
| `fan auto`    | Return to automatic fan control      |
| `reset`       | Software reset via SCB AIRCR         |

## Temperature → Fan Curve

```
Fan %
100 |                    ___________
    |                   /
 50 |          ________/
    |         /
  0 |________/
    +---+----+----+----+----> Temp (°C)
       30   45   60   80
             ↑         ↑
          WARNING   CRITICAL
```

## Technical Details

- **Clock**: HSE 8 MHz → PLL ×9 → 72 MHz SYSCLK
- **Flash latency**: 2 wait states + prefetch buffer
- **Bus clocks**: AHB 72 MHz, APB2 72 MHz, APB1 36 MHz, ADC 12 MHz
- **ADC**: 12-bit, software-triggered single conversion
- **PWM**: TIM3 CH1, 25 kHz, variable duty cycle
- **Sampling**: TIM2 interrupt at 10 Hz
- **Filter**: First-order IIR, α ≈ 0.125 (shift by 3)
- **Watchdog**: IWDG, ~2 second timeout, LSI 40 kHz / 64 prescaler

## License

Public domain — use however you like.
