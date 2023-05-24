/*
 * dht22.c
 *
 *      Created on: Apr 11, 2023
 *      Author: Akechi
 */

#include <drivers/dht22.h>

void DHT22_Init(void) {
    ROM_SysCtlPeripheralEnable(DHT22_GPIO_PERIPH);
    while(!ROM_SysCtlPeripheralReady(DHT22_GPIO_PERIPH));
}

void DHT22_Start(void) {
    GPIOPinTypeGPIOOutput(DHT22_PORT, DHT22_PIN);

    GPIOPinWrite(DHT22_PORT, DHT22_PIN, 0);             // pull the pin low
    DHT22_Delay(1300);                                  // wait for > 1ms

    GPIOPinTypeGPIOInput(DHT22_PORT, DHT22_PIN);
    ROM_GPIODirModeSet(DHT22_PORT, DHT22_PIN, GPIO_DIR_MODE_IN);
    ROM_GPIOPadConfigSet(DHT22_PORT, DHT22_PIN,
                                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
}

bool DHT22_Check_Response(void) {
    if (DHT22_Wait_Low(80)) {
        DHT22_Delay(120);                               // wait for 120us
        DHT22_Wait_Low(80);                             // wait for the pin to go low
        return true;
    }
    return false;
}

uint8_t DHT22_Read_Byte(void) {
    uint8_t byte = 0x00;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        DHT22_Wait_High(100);                           // wait for the pin to go high

        DHT22_Delay(40);                                // wait for 40us
        if (GPIOPinRead(DHT22_PORT, DHT22_PIN)) {
            byte |= (1<<(7-i));                         // write 1
        }
        else {
            byte &= ~(1<<(7-i));                        // write 0
        }

        DHT22_Wait_Low(100);                            // wait for the pin to go low
    }

    return byte;
}

DHT22_State_t DHT22_Read_Data(DHT22_Data_t *Data) {
    DHT22_Start();

    if (!DHT22_Check_Response()) return NOT_RESPONSE;   // DHT22 doesn't not response

    uint8_t RH_byte1 = DHT22_Read_Byte();
    uint8_t RH_byte2 = DHT22_Read_Byte();
    uint8_t T_byte1 = DHT22_Read_Byte();
    uint8_t T_byte2 = DHT22_Read_Byte();
    uint8_t checksum = DHT22_Read_Byte();

    uint8_t sum = (RH_byte1 + RH_byte2 + T_byte1 + T_byte2) & 0xFF;

    if (checksum != sum) return INVALID;                // data is invalid

    Data->humidity = (float)((RH_byte1 << 8) | RH_byte2) / 10.0;
    if (T_byte1 > 127) {
        Data->temperature = (float)((T_byte1 & 0x7F) << 8 | T_byte2) / 10.0 * (-1);
    }
    else {
        Data->temperature = (float)((T_byte1 << 8) | T_byte2) / 10.0;
    }

    return VALID;                                        // data is valid
}

bool DHT22_Wait_High(uint32_t timeout) {
    while (!GPIOPinRead(DHT22_PORT, DHT22_PIN)) {
        if(--timeout == 0) return false;
        DHT22_Delay(1);
    }
    return true;
}

bool DHT22_Wait_Low(uint32_t timeout) {
    while (GPIOPinRead(DHT22_PORT, DHT22_PIN)) {
        if (--timeout == 0) return false;
        DHT22_Delay(1);
    }
    return true;
}

void DHT22_Delay(uint32_t ui32Us) {
    ROM_SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

char* DHT22_To_String(float data, char *buffer) {
    char *str = buffer + DHT22_BUFFER_SIZE;              // go to end of buffer
    uint8_t decimals;                                    // variable to store the decimals
    uint8_t units;                                       // variable to store the part to left of decimal place

    if (data < 0) {
       decimals = (uint8_t)(data * (-10)) % 10;
       units = (uint8_t)(-1 * data);
    }
    else {
       decimals = (uint8_t)(data * 10) % 10;
       units = (uint8_t)data;
    }

    *str = '\0';
    *--str = decimals + '0';
    *--str = '.';

    while (units > 0) {
       *--str = units % 10 + '0';
       units /= 10;
    }
    if (data < 0) *--str = '-';                             // unary minus sign for negative numbers

    return str;
}
