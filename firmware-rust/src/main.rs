//! STM32F103 Thermal Monitor - Rust Implementation
//!
//! Modern Rust embedded firmware demonstrating:
//! - Type-safe peripheral access with PAC/HAL
//! - Zero-cost abstractions
//! - Ownership-based resource management
//! - Trait-based interfaces
//! - Safe interrupt handling with critical sections
//!
//! Hardware:
//! - STM32F103C8T6 (Blue Pill)
//! - External NTC thermistor on PA0 (ADC1_IN0)
//! - Internal temperature sensor (ADC1_IN16)
//! - PWM fan control on PA6 (TIM3_CH1)
//! - Status LED on PC13 (active low)
//! - UART telemetry on USART1 (PA9/PA10)

#![no_std]
#![no_main]

use panic_halt as _;

use cortex_m::peripheral::NVIC;
use cortex_m_rt::entry;
use stm32f1xx_hal::{
    adc::Adc,
    gpio::{gpioa::*, gpioc::*, Output, PushPull},
    pac::{interrupt, Peripherals, TIM2, TIM3},
    prelude::*,
    serial::{Config, Serial},
    timer::{CounterMs, Event, Tim3NoRemap},
};

use core::cell::{Cell, RefCell};
use core::fmt::Write;
use core::ops::DerefMut;
use cortex_m::interrupt::{free, Mutex};

/// Configuration constants
mod config {
    pub const TEMP_THRESHOLD_LOW_MC: i32 = 30_000;    // 30.0°C
    pub const TEMP_THRESHOLD_MID_MC: i32 = 45_000;    // 45.0°C
    pub const TEMP_THRESHOLD_HIGH_MC: i32 = 60_000;   // 60.0°C
    pub const TEMP_CRITICAL_MC: i32 = 80_000;         // 80.0°C

    pub const REPORT_INTERVAL_MS: u32 = 1000;
    pub const LED_BLINK_NORMAL_MS: u32 = 500;
    pub const LED_BLINK_WARN_MS: u32 = 200;
    pub const LED_BLINK_CRITICAL_MS: u32 = 50;

    pub const IIR_ALPHA_SHIFT: i32 = 3;
}

/// System state machine
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
enum SystemState {
    Init,
    Normal,
    Warning,
    Critical,
    Manual,
}

impl SystemState {
    fn as_str(&self) -> &'static str {
        match self {
            Self::Init => "INIT",
            Self::Normal => "NORMAL",
            Self::Warning => "WARNING",
            Self::Critical => "CRITICAL",
            Self::Manual => "MANUAL",
        }
    }
}

/// Temperature sensor with IIR filtering
struct TemperatureSensor {
    filtered_temp: i32,
}

impl TemperatureSensor {
    fn new(initial_temp: i32) -> Self {
        Self {
            filtered_temp: initial_temp,
        }
    }

    fn update(&mut self, raw_temp: i32) {
        self.filtered_temp += (raw_temp - self.filtered_temp) >> config::IIR_ALPHA_SHIFT;
    }

    fn get_temp(&self) -> i32 {
        self.filtered_temp
    }
}

/// Simple PWM duty cycle wrapper
struct PwmWrapper {
    max_duty: u16,
    current_duty: u8,
}

impl PwmWrapper {
    fn new(max_duty: u16) -> Self {
        Self {
            max_duty,
            current_duty: 0,
        }
    }

    fn set_duty(&mut self, duty_percent: u8) -> u16 {
        self.current_duty = duty_percent.min(100);
        (self.max_duty as u32 * self.current_duty as u32 / 100) as u16
    }

    fn get_duty(&self) -> u8 {
        self.current_duty
    }
}

/// Fan controller with automatic and manual modes
struct FanController {
    pwm_wrapper: PwmWrapper,
    manual_mode: bool,
}

impl FanController {
    fn new(max_duty: u16) -> Self {
        Self {
            pwm_wrapper: PwmWrapper::new(max_duty),
            manual_mode: false,
        }
    }

    fn update_auto(&mut self, temp_mc: i32) -> u16 {
        if self.manual_mode {
            return self.pwm_wrapper.set_duty(self.pwm_wrapper.get_duty());
        }

        let duty = Self::compute_duty(temp_mc);
        self.pwm_wrapper.set_duty(duty)
    }

    fn set_manual(&mut self, duty: u8) -> u16 {
        self.manual_mode = true;
        self.pwm_wrapper.set_duty(duty)
    }

    fn set_auto(&mut self) {
        self.manual_mode = false;
    }

    fn get_duty(&self) -> u8 {
        self.pwm_wrapper.get_duty()
    }

    fn is_manual(&self) -> bool {
        self.manual_mode
    }

    fn compute_duty(temp_mc: i32) -> u8 {
        use config::*;

        if temp_mc < TEMP_THRESHOLD_LOW_MC {
            0
        } else if temp_mc < TEMP_THRESHOLD_MID_MC {
            ((temp_mc - TEMP_THRESHOLD_LOW_MC) * 50
                / (TEMP_THRESHOLD_MID_MC - TEMP_THRESHOLD_LOW_MC)) as u8
        } else if temp_mc < TEMP_THRESHOLD_HIGH_MC {
            50 + ((temp_mc - TEMP_THRESHOLD_MID_MC) * 50
                / (TEMP_THRESHOLD_HIGH_MC - TEMP_THRESHOLD_MID_MC)) as u8
        } else {
            100
        }
    }
}

/// Convert NTC ADC reading to milli-degrees Celsius
fn ntc_raw_to_mc(raw: u16) -> i32 {
    let v_mv = (raw as i32 * 3300) / 4096;
    -30 * v_mv + 125_000
}

/// Global shared state for interrupt handling
static G_ADC_SAMPLE: Mutex<Cell<Option<u16>>> = Mutex::new(Cell::new(None));
static G_TIMER: Mutex<RefCell<Option<CounterMs<TIM2>>>> = Mutex::new(RefCell::new(None));
static G_PWM_CCR: Mutex<Cell<u16>> = Mutex::new(Cell::new(0));

#[entry]
fn main() -> ! {
    // Get peripherals
    let cp = cortex_m::Peripherals::take().unwrap();
    let dp = Peripherals::take().unwrap();

    // Setup clocks
    let mut flash = dp.FLASH.constrain();
    let rcc = dp.RCC.constrain();
    let clocks = rcc
        .cfgr
        .use_hse(8.MHz())
        .sysclk(72.MHz())
        .pclk1(36.MHz())
        .freeze(&mut flash.acr);

    // GPIO setup
    let mut gpioa = dp.GPIOA.split();
    let mut gpioc = dp.GPIOC.split();
    let mut afio = dp.AFIO.constrain();

    // Status LED (PC13, active low)
    let mut led = gpioc.pc13.into_push_pull_output(&mut gpioc.crh);
    led.set_high(); // Off initially

    // USART1 setup (PA9=TX, PA10=RX)
    let tx = gpioa.pa9.into_alternate_push_pull(&mut gpioa.crh);
    let rx = gpioa.pa10;
    let mut serial = Serial::new(
        dp.USART1,
        (tx, rx),
        &mut afio.mapr,
        Config::default().baudrate(115200.bps()),
        &clocks,
    );

    // Print banner
    writeln!(serial, "\r").ok();
    writeln!(serial, "========================================").ok();
    writeln!(serial, "  STM32F103 Thermal Monitor v3.0 (Rust)").ok();
    writeln!(serial, "  Modern Rust 2021 | 72 MHz | UART 115200").ok();
    writeln!(serial, "========================================").ok();
    writeln!(serial, "Commands: status, fan <0-100|auto>, reset\r").ok();
    writeln!(serial, "\r").ok();

    // ADC setup
    let mut adc = Adc::adc1(dp.ADC1, clocks);
    let mut adc_ch0 = gpioa.pa0.into_analog(&mut gpioa.crl);

    // PWM setup using TIM3 (PA6 = TIM3_CH1)
    let pa6 = gpioa.pa6.into_alternate_push_pull(&mut gpioa.crl);
    let mut pwm = dp.TIM3.pwm_hz::<Tim3NoRemap, _, _>(
        pa6,
        &mut afio.mapr,
        25.kHz(),
        &clocks,
    );
    let max_duty = pwm.get_max_duty();
    pwm.enable(stm32f1xx_hal::timer::Channel::C1);

    // Sampling timer (TIM2) - 10 Hz interrupt
    let mut timer = dp.TIM2.counter_ms(&clocks);
    timer.start(100.millis()).ok();
    timer.listen(Event::Update);

    // Store timer in global for ISR
    free(|cs| {
        G_TIMER.borrow(cs).replace(Some(timer));
    });

    // Enable TIM2 interrupt
    unsafe {
        NVIC::unmask(interrupt::TIM2);
    }

    // Application state
    let mut temp_ext = TemperatureSensor::new(25_000);
    let mut temp_int = TemperatureSensor::new(25_000);
    let mut fan = FanController::new(max_duty);
    let mut state = SystemState::Normal;

    let mut systick_delay = cp.SYST.delay(&clocks);
    let mut tick_count: u32 = 0;
    let mut last_report: u32 = 0;
    let mut last_blink: u32 = 0;

    // Main loop
    loop {
        // Process ADC sample if available
        if let Some(raw_ext) = free(|cs| G_ADC_SAMPLE.borrow(cs).take()) {
            // Convert and filter external temperature
            let raw_ext_mc = ntc_raw_to_mc(raw_ext);
            temp_ext.update(raw_ext_mc);

            // Read internal temperature sensor (returns i32 in milli-Celsius)
            let raw_int_mc: i32 = adc.read_temp();
            temp_int.update(raw_int_mc);

            // Update state and fan
            state = evaluate_state(temp_ext.get_temp(), fan.is_manual());
            let duty_value = fan.update_auto(temp_ext.get_temp());

            // Update PWM duty cycle
            pwm.set_duty(stm32f1xx_hal::timer::Channel::C1, duty_value);
        }

        // Periodic telemetry
        if tick_count.wrapping_sub(last_report) >= config::REPORT_INTERVAL_MS {
            last_report = tick_count;
            write!(serial, "[TMON] t={}s ext=", tick_count / 1000).ok();
            print_temp(&mut serial, temp_ext.get_temp());
            write!(serial, "C int=").ok();
            print_temp(&mut serial, temp_int.get_temp());
            write!(serial, "C fan={}% state={}\r\n",
                   fan.get_duty(), state.as_str()).ok();
        }

        // LED blink pattern
        let blink_period = match state {
            SystemState::Critical => config::LED_BLINK_CRITICAL_MS,
            SystemState::Warning => config::LED_BLINK_WARN_MS,
            _ => config::LED_BLINK_NORMAL_MS,
        };

        if tick_count.wrapping_sub(last_blink) >= blink_period {
            last_blink = tick_count;
            led.toggle();
        }

        // Simple delay and tick update
        systick_delay.delay(1.millis());
        tick_count = tick_count.wrapping_add(1);

        // Trigger ADC reading on timer tick
        free(|cs| {
            if let Some(ref mut timer) = G_TIMER.borrow(cs).borrow_mut().deref_mut() {
                if timer.wait().is_ok() {
                    let sample: u16 = adc.read(&mut adc_ch0).unwrap();
                    G_ADC_SAMPLE.borrow(cs).set(Some(sample));
                }
            }
        });
    }
}

fn evaluate_state(temp_mc: i32, is_manual: bool) -> SystemState {
    if is_manual {
        return SystemState::Manual;
    }
    if temp_mc >= config::TEMP_CRITICAL_MC {
        return SystemState::Critical;
    }
    if temp_mc >= config::TEMP_THRESHOLD_MID_MC {
        return SystemState::Warning;
    }
    SystemState::Normal
}

fn print_temp<W: Write>(writer: &mut W, temp_mc: i32) {
    let whole = temp_mc / 1000;
    let frac = (temp_mc.abs() % 1000) / 100;
    write!(writer, "{}.{}", whole, frac).ok();
}

#[interrupt]
fn TIM2() {
    // Timer interrupt is handled in main loop via wait()
    free(|cs| {
        if let Some(ref mut timer) = G_TIMER.borrow(cs).borrow_mut().deref_mut() {
            timer.clear_interrupt(Event::Update);
        }
    });
}
