#include "servo_pwm.h"
#include "rover_config.h"
#include "esp_log.h"

static const char *TAG = "servo_pwm";

/* Broad generic hobby-servo bound, independent of any rover-specific
 * clamping done upstream -- defense in depth, not the "real" safety bound. */
#define SERVO_PWM_ABS_MIN_US 500u
#define SERVO_PWM_ABS_MAX_US 2500u

#define SERVO_PWM_DUTY_RESOLUTION_BITS LEDC_TIMER_14_BIT
#define SERVO_PWM_DUTY_MAX ((1u << 14) - 1u) /* 16383 */
#define SERVO_PWM_TIMER LEDC_TIMER_0
#define SERVO_PWM_SPEED_MODE LEDC_LOW_SPEED_MODE /* only mode supported on ESP32-S3 */

static uint32_t pulse_us_to_duty(uint32_t pulse_us)
{
    /* 14-bit resolution over a 20000us period -> ~1.2207 us/count. */
    float duty = (float)pulse_us * (float)(SERVO_PWM_DUTY_MAX + 1) / (float)ROVER_SERVO_PERIOD_US;
    if (duty < 0.0f) {
        duty = 0.0f;
    } else if (duty > (float)SERVO_PWM_DUTY_MAX) {
        duty = (float)SERVO_PWM_DUTY_MAX;
    }
    return (uint32_t)(duty + 0.5f);
}

esp_err_t servo_pwm_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = SERVO_PWM_SPEED_MODE,
        .duty_resolution = SERVO_PWM_DUTY_RESOLUTION_BITS,
        .timer_num = SERVO_PWM_TIMER,
        .freq_hz = 1000000u / ROVER_SERVO_PERIOD_US, /* 50 Hz */
        .clk_cfg = LEDC_AUTO_CLK,
    };
    return ledc_timer_config(&timer_cfg);
}

esp_err_t servo_pwm_attach(gpio_num_t gpio_num, ledc_channel_t channel,
                            uint32_t initial_pulse_us)
{
    if (initial_pulse_us < SERVO_PWM_ABS_MIN_US) {
        initial_pulse_us = SERVO_PWM_ABS_MIN_US;
    } else if (initial_pulse_us > SERVO_PWM_ABS_MAX_US) {
        initial_pulse_us = SERVO_PWM_ABS_MAX_US;
    }

    ledc_channel_config_t channel_cfg = {
        .gpio_num = gpio_num,
        .speed_mode = SERVO_PWM_SPEED_MODE,
        .channel = channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_PWM_TIMER,
        .duty = pulse_us_to_duty(initial_pulse_us),
        .hpoint = 0,
    };
    esp_err_t err = ledc_channel_config(&channel_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed for GPIO%d: %s", gpio_num, esp_err_to_name(err));
    }
    /* ledc_channel_config() latches the initial duty itself; once set, the
     * LEDC hardware free-runs the 50Hz waveform with zero further CPU
     * involvement -- this is why the datasheet's "refresh every 20ms"
     * requirement is satisfied automatically after this call returns. */
    return err;
}
