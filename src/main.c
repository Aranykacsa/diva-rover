/*
 * Full-gas, no-safety-net test: drives both wheels to 100% speed the
 * instant the board boots and holds it there indefinitely. Deliberately
 * bypasses rover_drive.c (arm/disarm, watchdog, slew-rate ramp, e-stop) --
 * that file is left untouched and unused so this is easy to revert later.
 * The only way to stop the rover once this is flashed is to cut power.
 *
 * Datasheet note (#900-00008): no stall current is specified for this
 * servo. If a wheel jams against an obstacle while commanded full speed,
 * the motor will keep drawing whatever elevated current that causes,
 * indefinitely -- nothing in this build detects or reacts to that.
 *
 * Hardware wiring reminder: servo signal -> GPIO4 (left) / GPIO5 (right)
 * directly (3.3V is within the servo's accepted input range). Servo motor
 * power (Vservo, 4-6VDC) must come from a separate, correctly-rated
 * supply, never from this board's own regulator.
 */

#include "servo_pwm.h"
#include "rover_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "rover_full_gas";

/* Same pulse math as rover_drive.c's control task, evaluated once at
 * speed = +1.0 (full forward) so this stays in sync with whatever
 * invert/trim tuning is current in rover_config.h. */
static uint32_t full_speed_pulse_us(int invert, int trim_us)
{
    return (uint32_t)((int32_t)ROVER_SERVO_NEUTRAL_US + invert * (int32_t)ROVER_SERVO_HALF_RANGE_US + trim_us);
}

void app_main(void)
{
    uint32_t left_pulse = full_speed_pulse_us(ROVER_LEFT_INVERT, ROVER_LEFT_TRIM_US);
    uint32_t right_pulse = full_speed_pulse_us(ROVER_RIGHT_INVERT, ROVER_RIGHT_TRIM_US);

    ESP_ERROR_CHECK(servo_pwm_init());
    ESP_ERROR_CHECK(servo_pwm_attach(ROVER_LEFT_SERVO_GPIO, LEDC_CHANNEL_0, left_pulse));
    ESP_ERROR_CHECK(servo_pwm_attach(ROVER_RIGHT_SERVO_GPIO, LEDC_CHANNEL_1, right_pulse));

    ESP_LOGI(TAG, "full gas: left=%uus right=%uus -- no e-stop, no watchdog, no ramp",
             (unsigned)left_pulse, (unsigned)right_pulse);

    /* Nothing left to do: the LEDC hardware free-runs this duty cycle on
     * its own from here on, with zero further CPU involvement. */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
