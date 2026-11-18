#pragma once

#include "driver/gpio.h"

/* ---- Pin assignment ---- */
#define ROVER_LEFT_SERVO_GPIO   GPIO_NUM_4
#define ROVER_RIGHT_SERVO_GPIO  GPIO_NUM_5

/* ---- Servo pulse timing (Parallax #900-00008 datasheet) ----
 * Full range is 1300us (full speed CW) to 1700us (full speed CCW); 1500us
 * is neutral. HALF_RANGE_US is the +-200us deviation "full speed" is. */
#define ROVER_SERVO_PERIOD_US      20000u   /* 50 Hz refresh */
#define ROVER_SERVO_NEUTRAL_US     1500u    /* nominal center / no rotation */
#define ROVER_SERVO_HALF_RANGE_US   200u

/* ---- Per-wheel software trim, added to NEUTRAL_US.
 * Tune only after the servo's own mechanical trim pot has been centered.
 * ROVER_RIGHT_TRIM_US=-20 added 2026-07-24: right side (both right servos,
 * which share one PWM signal) was observed spinning visibly faster than
 * left at full speed. Pulling right's pulse back toward center by 20us
 * trims its speed down rather than pushing left beyond the datasheet's
 * documented full-speed endpoint. Iterate this value on hardware: more
 * negative if right is still faster, less negative if it overshoots. */
#define ROVER_LEFT_TRIM_US    0
#define ROVER_RIGHT_TRIM_US   (-20)

/* ---- Per-wheel mounting polarity. Servos are mirrored on a differential
 * chassis, so a shared "forward" command must drive opposite pulse-width
 * deviation on the two sides for the wheels to push the chassis the same
 * way. Verified on hardware 2026-07-23: with equal signs (+1,+1), the
 * right wheel drove forward and the left wheel drove backward (the rover
 * span in place) -- confirming this mount needs opposite signs after all.
 * Right's sense was correct, so left is the one flipped here. If forward
 * motion still isn't straight, flip LEFT back and flip RIGHT instead. */
#define ROVER_LEFT_INVERT   (-1)
#define ROVER_RIGHT_INVERT  (+1)
