/**
 * Thermal Monitor — Main Application (C++)
 *
 * Modern C++17 implementation of bare-metal STM32F103C8T6 thermal monitoring:
 *   - Type-safe peripheral wrappers with RAII
 *   - Object-oriented architecture
 *   - Template-based configurations
 *   - Strong type system for hardware abstraction
 *
 * Features:
 *   - ADC monitoring (external NTC + internal temp sensor)
 *   - IIR digital filtering for noise rejection
 *   - PWM fan control with automatic duty cycling
 *   - UART telemetry and command interface
 *   - State machine with LED status indicators
 *   - Watchdog timer for reliability
 */

#include "stm32f103.hpp"
#include "system.hpp"
#include "gpio.hpp"
#include "uart.hpp"
#include "adc.hpp"
#include "timer.hpp"
#include <array>
#include <string_view>
#include <optional>
#include <cstring>

namespace config {
    constexpr int32_t TEMP_THRESHOLD_LOW_MC    = 30000;   // 30.0 °C
    constexpr int32_t TEMP_THRESHOLD_MID_MC    = 45000;   // 45.0 °C
    constexpr int32_t TEMP_THRESHOLD_HIGH_MC   = 60000;   // 60.0 °C
    constexpr int32_t TEMP_CRITICAL_MC         = 80000;   // 80.0 °C

    constexpr uint32_t REPORT_INTERVAL_MS      = 1000;
    constexpr uint32_t LED_BLINK_NORMAL_MS     = 500;
    constexpr uint32_t LED_BLINK_WARN_MS       = 200;
    constexpr uint32_t LED_BLINK_CRITICAL_MS   = 50;

    constexpr uint8_t IIR_ALPHA_SHIFT          = 3;

    constexpr uint32_t FAN_PWM_FREQ_HZ         = 25000;
    constexpr uint32_t SAMPLE_RATE_HZ          = 10;
}

/**
 * System state enumeration
 */
enum class SystemState : uint8_t {
    Init,
    Normal,
    Warning,
    Critical,
    Manual
};

constexpr std::array<std::string_view, 5> STATE_NAMES = {
    "INIT", "NORMAL", "WARNING", "CRITICAL", "MANUAL"
};

/**
 * Temperature sensor with IIR filtering
 */
class TemperatureSensor {
public:
    explicit TemperatureSensor(int32_t initial_temp = 25000)
        : filtered_temp_(initial_temp) {}

    void update(int32_t raw_temp) {
        filtered_temp_ += (raw_temp - filtered_temp_) >> config::IIR_ALPHA_SHIFT;
    }

    int32_t get_temp() const { return filtered_temp_; }

private:
    int32_t filtered_temp_;
};

/**
 * Fan controller with automatic and manual modes
 */
class FanController {
public:
    FanController() : duty_(0), manual_mode_(false) {}

    void update_auto(int32_t temp_mc) {
        if (manual_mode_) return;

        duty_ = compute_duty(temp_mc);
        timer::pwm_set_duty(duty_);
    }

    void set_manual(uint8_t duty) {
        manual_mode_ = true;
        duty_ = (duty > 100) ? 100 : duty;
        timer::pwm_set_duty(duty_);
    }

    void set_auto() {
        manual_mode_ = false;
    }

    uint8_t get_duty() const { return duty_; }
    bool is_manual() const { return manual_mode_; }

private:
    uint8_t duty_;
    bool manual_mode_;

    static uint8_t compute_duty(int32_t temp_mc) {
        if (temp_mc < config::TEMP_THRESHOLD_LOW_MC) {
            return 0;
        } else if (temp_mc < config::TEMP_THRESHOLD_MID_MC) {
            return static_cast<uint8_t>(
                ((temp_mc - config::TEMP_THRESHOLD_LOW_MC) * 50) /
                (config::TEMP_THRESHOLD_MID_MC - config::TEMP_THRESHOLD_LOW_MC)
            );
        } else if (temp_mc < config::TEMP_THRESHOLD_HIGH_MC) {
            return 50 + static_cast<uint8_t>(
                ((temp_mc - config::TEMP_THRESHOLD_MID_MC) * 50) /
                (config::TEMP_THRESHOLD_HIGH_MC - config::TEMP_THRESHOLD_MID_MC)
            );
        }
        return 100;
    }
};

/**
 * Watchdog timer wrapper
 */
class Watchdog {
public:
    Watchdog() {
        stm32::IWDG->KR  = stm32::iwdg::KEY_ACCESS;
        stm32::IWDG->PR  = 4;        // Prescaler /64
        stm32::IWDG->RLR = 1250;     // ~2 seconds timeout
        stm32::IWDG->KR  = stm32::iwdg::KEY_ENABLE;
    }

    void feed() {
        stm32::IWDG->KR = stm32::iwdg::KEY_RELOAD;
    }
};

/**
 * Thermal monitoring application
 */
class ThermalMonitor {
public:
    ThermalMonitor()
        : status_led_(stm32::GPIOC, 13),
          sample_ready_(false),
          adc_raw_ext_(0),
          state_(SystemState::Init)
    {
        initialize_hardware();
        print_banner();
        state_ = SystemState::Normal;
    }

    void run() {
        uint32_t last_report = 0;
        uint32_t last_blink = 0;

        while (true) {
            uint32_t now = system::get_tick();

            process_samples();
            handle_telemetry(now, last_report);
            handle_led_blink(now, last_blink);
            process_commands();

            watchdog_.feed();
        }
    }

    // ISR callback
    void on_sample_timer() {
        adc_raw_ext_ = adc::read(0);
        sample_ready_ = true;
    }

private:
    gpio::Pin status_led_;
    TemperatureSensor temp_ext_;
    TemperatureSensor temp_int_;
    FanController fan_;
    Watchdog watchdog_;

    volatile bool sample_ready_;
    volatile uint16_t adc_raw_ext_;
    SystemState state_;

    void initialize_hardware() {
        system::init_clock();
        gpio::init();
        uart::init(115200);
        adc::init();
        timer::pwm_init(config::FAN_PWM_FREQ_HZ);
        timer::sampling_timer_init(config::SAMPLE_RATE_HZ);

        status_led_.set_mode(gpio::Mode::Output_2MHz, gpio::Config::OutputPushPull);
        status_led_.write(gpio::State::High);  // LED off (active low)
    }

    void print_banner() {
        auto& uart = uart::instance();
        uart.put_string("\n");
        uart.put_string("========================================\n");
        uart.put_string("  STM32F103 Thermal Monitor v2.0 (C++)\n");
        uart.put_string("  Modern C++17 | 72 MHz | UART 115200\n");
        uart.put_string("========================================\n");
        uart.put_string("Commands: status, fan <0-100|auto>, reset\n\n");
    }

    void process_samples() {
        if (!sample_ready_) return;

        sample_ready_ = false;

        // Convert and filter external temperature
        int32_t raw_ext_mc = ntc_raw_to_mc(adc_raw_ext_);
        temp_ext_.update(raw_ext_mc);

        // Read and filter internal temperature
        int32_t raw_int_mc = adc::read_temperature_mc();
        temp_int_.update(raw_int_mc);

        // Update state and fan
        state_ = evaluate_state(temp_ext_.get_temp());
        fan_.update_auto(temp_ext_.get_temp());
    }

    void handle_telemetry(uint32_t now, uint32_t& last_report) {
        if ((now - last_report) < config::REPORT_INTERVAL_MS) return;

        last_report = now;
        auto& uart = uart::instance();

        uart.put_string("[TMON] t=");
        uart.print_int(now / 1000);
        uart.put_string("s ext=");
        print_temp(uart, temp_ext_.get_temp());
        uart.put_string("C int=");
        print_temp(uart, temp_int_.get_temp());
        uart.put_string("C fan=");
        uart.print_int(fan_.get_duty());
        uart.put_string("% state=");
        uart.put_string(STATE_NAMES[static_cast<uint8_t>(state_)]);
        uart.put_char('\n');
    }

    void handle_led_blink(uint32_t now, uint32_t& last_blink) {
        uint32_t blink_period = get_blink_period();
        if ((now - last_blink) < blink_period) return;

        last_blink = now;
        status_led_.toggle();
    }

    void process_commands() {
        auto& uart = uart::instance();
        if (!uart.rx_available()) return;

        char cmd_buf[32];
        uint16_t len = uart.read_line(cmd_buf, sizeof(cmd_buf));
        if (len > 0) {
            handle_command(std::string_view(cmd_buf, len));
        }
    }

    void handle_command(std::string_view cmd) {
        auto& uart = uart::instance();

        if (cmd == "status") {
            uart.put_string("\n=== THERMAL MONITOR STATUS ===\n");
            uart.put_string("State:    ");
            uart.put_string(STATE_NAMES[static_cast<uint8_t>(state_)]);
            uart.put_char('\n');
            uart.put_string("Ext Temp: ");
            print_temp(uart, temp_ext_.get_temp());
            uart.put_string(" C\n");
            uart.put_string("Int Temp: ");
            print_temp(uart, temp_int_.get_temp());
            uart.put_string(" C\n");
            uart.put_string("Fan:      ");
            uart.print_int(fan_.get_duty());
            uart.put_string("%\n");
            uart.put_string("Uptime:   ");
            uart.print_int(system::get_tick() / 1000);
            uart.put_string(" s\n");
            uart.put_string("==============================\n");
        }
        else if (cmd.starts_with("fan ")) {
            handle_fan_command(cmd.substr(4));
        }
        else if (cmd == "reset") {
            uart.put_string("Resetting...\n");
            system::delay_ms(100);
            stm32::SCB->AIRCR = stm32::scb::AIRCR_VECTKEY |
                               stm32::scb::AIRCR_SYSRESETREQ;
            while (true) {}
        }
        else if (!cmd.empty()) {
            uart.put_string("Unknown command: ");
            uart.put_string(cmd);
            uart.put_string("\nCommands: status, fan <0-100|auto>, reset\n");
        }
    }

    void handle_fan_command(std::string_view arg) {
        auto& uart = uart::instance();

        if (arg == "auto") {
            fan_.set_auto();
            uart.put_string("Fan: AUTO mode\n");
        } else {
            int val = parse_int(arg);
            fan_.set_manual(static_cast<uint8_t>(val));
            uart.put_string("Fan: MANUAL ");
            uart.print_int(val);
            uart.put_string("%\n");
        }
    }

    SystemState evaluate_state(int32_t temp_mc) const {
        if (fan_.is_manual()) return SystemState::Manual;
        if (temp_mc >= config::TEMP_CRITICAL_MC) return SystemState::Critical;
        if (temp_mc >= config::TEMP_THRESHOLD_MID_MC) return SystemState::Warning;
        return SystemState::Normal;
    }

    uint32_t get_blink_period() const {
        switch (state_) {
        case SystemState::Critical: return config::LED_BLINK_CRITICAL_MS;
        case SystemState::Warning:  return config::LED_BLINK_WARN_MS;
        default:                    return config::LED_BLINK_NORMAL_MS;
        }
    }

    static int32_t ntc_raw_to_mc(uint16_t raw) {
        int32_t v_mv = (static_cast<int32_t>(raw) * 3300) / 4096;
        return -30 * v_mv + 125000;
    }

    static void print_temp(uart::UART& uart, int32_t temp_mc) {
        uart.print_int(temp_mc / 1000);
        uart.put_char('.');
        int32_t frac = (temp_mc % 1000) / 100;
        if (frac < 0) frac = -frac;
        uart.print_int(frac);
    }

    static int parse_int(std::string_view str) {
        int val = 0;
        for (char c : str) {
            if (c >= '0' && c <= '9') {
                val = val * 10 + (c - '0');
            }
        }
        return (val > 100) ? 100 : val;
    }
};

// Global instance for ISR access
static ThermalMonitor* g_monitor = nullptr;

extern "C" void TIM2_IRQHandler() {
    if (stm32::TIM2->SR & stm32::tim::SR_UIF) {
        stm32::TIM2->SR &= ~stm32::tim::SR_UIF;
        if (g_monitor) {
            g_monitor->on_sample_timer();
        }
    }
}

int main() {
    ThermalMonitor monitor;
    g_monitor = &monitor;
    monitor.run();
    return 0;
}
