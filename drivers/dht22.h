/*
 * dht22.h
 *
 *      Created on: Apr 11, 2023
 *      Author: Akechi
 */

#ifndef DRIVERS_DHT22_H_
#define DRIVERS_DHT22_H_

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#define     DHT22_PIN               GPIO_PIN_2
#define     DHT22_PORT              GPIO_PORTA_BASE
#define     DHT22_GPIO_PERIPH       SYSCTL_PERIPH_GPIOA
#define     DHT22_BUFFER_SIZE       6

typedef enum {NOT_RESPONSE=0, INVALID, VALID} DHT22_State_t;

typedef struct {
    float temperature;
    float humidity;
} DHT22_Data_t;

void            DHT22_Init(void);
void            DHT22_Start(void);
bool            DHT22_Check_Response(void);
uint8_t         DHT22_Read_Byte(void);
DHT22_State_t   DHT22_Read_Data(DHT22_Data_t *Data);
bool            DHT22_Wait_High(uint32_t timeout);
bool            DHT22_Wait_Low(uint32_t timeout);
void            DHT22_Delay(uint32_t ui32Us);
char*           DHT22_To_String(float data, char *buffer);

#endif /* DRIVERS_DHT22_H_ */
