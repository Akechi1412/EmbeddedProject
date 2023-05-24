/*
 * servo.h
 *
 *  Created on: May 22, 2023
 *      Author: Akechi
 */

#ifndef DRIVERS_SERVO_H_
#define DRIVERS_SERVO_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"

#define     SERVO_PIN               GPIO_PIN_0
#define     SERVO_PORT              GPIO_PORTD_BASE
#define     SERVO_GPIO_PERIPH       SYSCTL_PERIPH_GPIOD
#define     SERVO_PIN_CONFIGURE     GPIO_PD0_M1PWM0
#define     PWM_FREQUENCY           50

void            Servo_Init(void);
void            Servo_Control(uint8_t angle);

#endif /* DRIVERS_SERVO_H_ */
