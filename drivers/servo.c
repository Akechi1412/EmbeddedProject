/*
 * servo.c
 *
 *  Created on: May 22, 2023
 *      Author: Akechi
 */

#include <drivers/servo.h>

volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint8_t ui8Adjust;

void Servo_Init(void) {
    ui8Adjust = 75;

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SERVO_GPIO_PERIPH);
    ROM_GPIOPinTypePWM(SERVO_PORT, SERVO_PIN);
    ROM_GPIOPinConfigure(SERVO_PIN_CONFIGURE);
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);
}

void Servo_Control(uint8_t angle) {
    if (angle > 180) {
        angle = 180;
    }

    ui8Adjust = (uint8_t)(angle / 180.0 * (125 - 25) + 25);


    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
}




