#pragma once

#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

/* Generic hobby-servo PWM driver on top of the ESP-IDF LEDC peripheral.
 * Knows nothing about "rover", wheels, arming, trim, or direction -- just
 * "drive this channel to pulse width X microseconds, forever, in hardware." */

/* Configures LEDC_TIMER_0 for 50Hz / 14-bit duty resolution, low-speed mode.
 * Call once before any servo_pwm_attach(). */
esp_err_t servo_pwm_init(void);

/* Binds one LEDC channel to a GPIO on the shared timer and immediately
 * latches initial_pulse_us so a defined pulse exists before this returns.
 * Hard-clamps initial_pulse_us to a broad generic hobby-servo bound,
 * regardless of any caller-side clamping. */
esp_err_t servo_pwm_attach(gpio_num_t gpio_num, ledc_channel_t channel,
                            uint32_t initial_pulse_us);
