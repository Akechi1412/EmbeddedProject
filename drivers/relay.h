/*
 * relay.h
 *
 *  Created on: May 22, 2023
 *      Author: Akechi
 */

#ifndef DRIVERS_RELAY_H_
#define DRIVERS_RELAY_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#define     RELAY_PIN               GPIO_PIN_4
#define     RELAY_PORT              GPIO_PORTA_BASE
#define     RELAY_GPIO_PERIPH       SYSCTL_PERIPH_GPIOA

enum relayState  {ON=0,OFF=1};

void            Relay_Init(void);
void            Relay_Control(enum relayState State);

#endif /* DRIVERS_RELAY_H_ */
