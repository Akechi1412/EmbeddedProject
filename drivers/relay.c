/*
 * relay.c
 *
 *  Created on: May 22, 2023
 *      Author: Akechi
 */

#include <drivers/relay.h>

void Relay_Init(void) {
    ROM_SysCtlPeripheralEnable(RELAY_GPIO_PERIPH);
    ROM_GPIOPinTypeGPIOOutputOD(RELAY_PORT, RELAY_PIN);
}

void Relay_Control(enum relayState State) {
    if (State == ON) {
        GPIOPinWrite(RELAY_PORT, RELAY_PIN, 0);
    }
    else {
        GPIOPinWrite(RELAY_PORT, RELAY_PIN, RELAY_PIN);
    }
}


