/**
 * Thermal Monitor — Main Application
 *
 * Bare-metal STM32F103C8T6 firmware:
 *   - Reads ADC ch0 (external NTC thermistor) and ch16 (internal temp sensor)
 *   - Applies IIR low-pass digital filter to ADC readings
 *   - Controls cooling fan via PWM based on temperature thresholds
 *   - Reports telemetry over UART1 at 115200 baud
 *   - Blinks status LED (PC13) with pattern indicating system state
 *   - Accepts UART commands: "status", "fan <0-100>", "reset"
 *   - Watchdog timer for reliability
 *
 * Pin assignments:
 *   PC13 — Status LED (active low on Blue Pill)
 *   PA0  — ADC ch0 (external temperature sensor, 0..3.3V)
 *   PA6  — TIM3 CH1 PWM (fan control)
 *   PA9  — USART1 TX
 *   PA10 — USART1 RX
 */

#include "stm32f103.h"
#include "system.h"
#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "timer.h"
#include <string.h>
#include <stdbool.h>

/* ---- Configuration ---- */
#define TEMP_THRESHOLD_LOW_MC    30000   /* 30.0 °C — fan off         */
#define TEMP_THRESHOLD_MID_MC    45000   /* 45.0 °C — fan 50%         */
#define TEMP_THRESHOLD_HIGH_MC   60000   /* 60.0 °C — fan 100%        */
#define TEMP_CRITICAL_MC         80000   /* 80.0 °C — alarm state     */

#define REPORT_INTERVAL_MS       1000    /* Telemetry period           */
#define LED_BLINK_NORMAL_MS      500     /* Normal blink rate          */
#define LED_BLINK_WARN_MS        200     /* Warning blink rate         */
#define LED_BLINK_CRITICAL_MS    50      /* Critical fast blink        */

#define IIR_ALPHA_SHIFT          3       /* Filter: new = old + (raw - old) >> 3 */

#define FAN_PWM_FREQ_HZ          25000   /* 25 kHz PWM for fan        */
#define SAMPLE_RATE_HZ           10      /* ADC sample rate            */

/* ---- System state machine ---- */
typedef enum {
    STATE_INIT,
    STATE_NORMAL,
    STATE_WARNING,
    STATE_CRITICAL,
    STATE_MANUAL,
} sys_state_t;

static const char *state_names[] = {
    "INIT", "NORMAL", "WARNING", "CRITICAL", "MANUAL"
};

/* ---- Global state ---- */
static volatile bool     g_sample_ready = false;
static volatile uint16_t g_adc_raw_ext  = 0;

static int32_t  g_temp_ext_mc   = 25000;  /* Filtered external temp (m°C) */
static int32_t  g_temp_int_mc   = 25000;  /* Filtered internal temp (m°C) */
static uint8_t  g_fan_duty      = 0;
static bool     g_fan_manual    = false;

static sys_state_t g_state = STATE_INIT;

/* ---- IIR low-pass filter ---- */
static int32_t iir_filter(int32_t prev, int32_t raw) {
    return prev + ((raw - prev) >> IIR_ALPHA_SHIFT);
}

/* ---- Convert NTC ADC reading to milli-degrees Celsius ---- */
/*
 * Simplified linearized model for a 10K NTC with 3.3V reference:
 *   V = raw * 3300 / 4096 (mV)
 *   Using Steinhart-Hart approximation (linearized around 25°C):
 *     T(m°C) ≈ -30 * V_mv + 125000
 *   This is a rough model — real firmware would use a lookup table.
 */
static int32_t ntc_raw_to_mc(uint16_t raw) {
    int32_t v_mv = ((int32_t)raw * 3300) / 4096;
    return -30 * v_mv + 125000;
}

/* ---- Compute fan duty from temperature ---- */
static uint8_t compute_fan_duty(int32_t temp_mc) {
    if (temp_mc < TEMP_THRESHOLD_LOW_MC) {
        return 0;
    } else if (temp_mc < TEMP_THRESHOLD_MID_MC) {
        /* Linear ramp 0% -> 50% between LOW and MID */
        return (uint8_t)(((temp_mc - TEMP_THRESHOLD_LOW_MC) * 50)
                / (TEMP_THRESHOLD_MID_MC - TEMP_THRESHOLD_LOW_MC));
    } else if (temp_mc < TEMP_THRESHOLD_HIGH_MC) {
        /* Linear ramp 50% -> 100% between MID and HIGH */
        return 50 + (uint8_t)(((temp_mc - TEMP_THRESHOLD_MID_MC) * 50)
                / (TEMP_THRESHOLD_HIGH_MC - TEMP_THRESHOLD_MID_MC));
    } else {
        return 100;
    }
}

/* ---- Determine system state ---- */
static sys_state_t evaluate_state(int32_t temp_mc) {
    if (g_fan_manual) return STATE_MANUAL;
    if (temp_mc >= TEMP_CRITICAL_MC) return STATE_CRITICAL;
    if (temp_mc >= TEMP_THRESHOLD_MID_MC) return STATE_WARNING;
    return STATE_NORMAL;
}

/* ---- UART command parser ---- */
static void process_command(const char *cmd) {
    if (strcmp(cmd, "status") == 0) {
        uart_puts("\n=== THERMAL MONITOR STATUS ===\n");
        uart_puts("State:    "); uart_puts(state_names[g_state]); uart_putc('\n');
        uart_puts("Ext Temp: "); uart_print_int(g_temp_ext_mc / 1000);
        uart_putc('.'); uart_print_int((g_temp_ext_mc % 1000) / 100); uart_puts(" C\n");
        uart_puts("Int Temp: "); uart_print_int(g_temp_int_mc / 1000);
        uart_putc('.'); uart_print_int((g_temp_int_mc % 1000) / 100); uart_puts(" C\n");
        uart_puts("Fan:      "); uart_print_int(g_fan_duty); uart_puts("%\n");
        uart_puts("Uptime:   "); uart_print_int((int32_t)(get_tick() / 1000)); uart_puts(" s\n");
        uart_puts("==============================\n");
    }
    else if (strncmp(cmd, "fan ", 4) == 0) {
        /* Parse duty cycle: "fan 75" or "fan auto" */
        if (strcmp(cmd + 4, "auto") == 0) {
            g_fan_manual = false;
            uart_puts("Fan: AUTO mode\n");
        } else {
            int val = 0;
            const char *p = cmd + 4;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val > 100) val = 100;
            g_fan_manual = true;
            g_fan_duty = (uint8_t)val;
            pwm_set_duty(g_fan_duty);
            uart_puts("Fan: MANUAL ");
            uart_print_int(val);
            uart_puts("%\n");
        }
    }
    else if (strcmp(cmd, "reset") == 0) {
        uart_puts("Resetting...\n");
        delay_ms(100);
        SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESET;
        while (1);
    }
    else if (cmd[0] != '\0') {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\nCommands: status, fan <0-100|auto>, reset\n");
    }
}

/* ---- TIM2 ISR: periodic ADC sampling ---- */
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        g_adc_raw_ext = adc_read(0);
        g_sample_ready = true;
    }
}

/* ---- Print telemetry line ---- */
static void print_telemetry(void) {
    uart_puts("[TMON] t=");
    uart_print_int((int32_t)(get_tick() / 1000));
    uart_puts("s ext=");
    uart_print_int(g_temp_ext_mc / 1000);
    uart_putc('.');
    /* Print one decimal place (absolute value for fractional part) */
    int32_t frac = (g_temp_ext_mc % 1000) / 100;
    if (frac < 0) frac = -frac;
    uart_print_int(frac);
    uart_puts("C int=");
    uart_print_int(g_temp_int_mc / 1000);
    uart_putc('.');
    frac = (g_temp_int_mc % 1000) / 100;
    if (frac < 0) frac = -frac;
    uart_print_int(frac);
    uart_puts("C fan=");
    uart_print_int(g_fan_duty);
    uart_puts("% state=");
    uart_puts(state_names[g_state]);
    uart_putc('\n');
}

/* ---- Watchdog init ---- */
static void watchdog_init(void) {
    IWDG->KR  = IWDG_KEY_ACCESS;   /* Unlock registers  */
    IWDG->PR  = 4;                  /* Prescaler /64     */
    IWDG->RLR = 1250;              /* ~2 seconds timeout (40 kHz / 64 * 1250 = 2s) */
    IWDG->KR  = IWDG_KEY_ENABLE;   /* Start watchdog    */
}

static void watchdog_feed(void) {
    IWDG->KR = IWDG_KEY_RELOAD;
}

/* ================================================================ */
/*                              MAIN                                */
/* ================================================================ */

int main(void) {
    /* --- Hardware initialization --- */
    gpio_init();
    uart_init(115200);
    adc_init();
    pwm_init(FAN_PWM_FREQ_HZ);
    sampling_timer_init(SAMPLE_RATE_HZ);

    /* Status LED: PC13 output push-pull (active low on Blue Pill) */
    gpio_set_mode(GPIOC, 13, PIN_MODE_OUTPUT_PP_2MHZ);
    gpio_write(GPIOC, 13, 1);  /* LED off */

    /* Enable watchdog */
    watchdog_init();

    /* --- Startup banner --- */
    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  STM32F103 Thermal Monitor v1.0\n");
    uart_puts("  Bare-metal | 72 MHz | UART 115200\n");
    uart_puts("========================================\n");
    uart_puts("Commands: status, fan <0-100|auto>, reset\n\n");

    g_state = STATE_NORMAL;

    uint32_t last_report = 0;
    uint32_t last_blink  = 0;
    char cmd_buf[32];

    /* --- Main loop --- */
    while (1) {
        uint32_t now = get_tick();

        /* --- Process ADC samples --- */
        if (g_sample_ready) {
            g_sample_ready = false;

            /* Convert and filter external temperature */
            int32_t raw_ext_mc = ntc_raw_to_mc(g_adc_raw_ext);
            g_temp_ext_mc = iir_filter(g_temp_ext_mc, raw_ext_mc);

            /* Read and filter internal temperature (less frequent) */
            int32_t raw_int_mc = adc_read_temperature_mc();
            g_temp_int_mc = iir_filter(g_temp_int_mc, raw_int_mc);

            /* Update state and fan (unless manual override) */
            g_state = evaluate_state(g_temp_ext_mc);
            if (!g_fan_manual) {
                g_fan_duty = compute_fan_duty(g_temp_ext_mc);
                pwm_set_duty(g_fan_duty);
            }
        }

        /* --- Periodic telemetry report --- */
        if ((now - last_report) >= REPORT_INTERVAL_MS) {
            last_report = now;
            print_telemetry();
        }

        /* --- LED blink pattern based on state --- */
        uint32_t blink_period;
        switch (g_state) {
        case STATE_CRITICAL: blink_period = LED_BLINK_CRITICAL_MS; break;
        case STATE_WARNING:  blink_period = LED_BLINK_WARN_MS;     break;
        default:             blink_period = LED_BLINK_NORMAL_MS;   break;
        }
        if ((now - last_blink) >= blink_period) {
            last_blink = now;
            gpio_toggle(GPIOC, 13);
        }

        /* --- Process UART commands --- */
        if (uart_rx_available()) {
            uint16_t len = uart_read_line(cmd_buf, sizeof(cmd_buf));
            if (len > 0) {
                process_command(cmd_buf);
            }
        }

        /* --- Feed watchdog --- */
        watchdog_feed();
    }

    return 0; /* Never reached */
}
