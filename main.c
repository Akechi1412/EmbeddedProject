
/*
 * main.c
 *
 *  Created on: May 22, 2023
 *      Author: Akechi
 */


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include <drivers/dht22.h>
#include <drivers/servo.h>
#include <drivers/relay.h>

#define BUFFER_SIZE 100

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif


//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void) {
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

char buffer[BUFFER_SIZE];
uint8_t index;

typedef struct {
    char type;
    char value;
} Data_t;

xSemaphoreHandle    xBinarySemaphore;
xQueueHandle        xQueue;
static void SensorTask(void *pvParameters);
static void vSenderTask(void *pvParameters);
static void vReceiverTask(void *pvParameters);

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                           SYSCTL_OSC_MAIN);

    // Config UART1 to communication with ESP32-CAM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    IntMasterEnable();
    IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RT | UART_INT_RX);
//    IntPrioritySet(UART1_BASE, 6 << 5);


    ConfigureUART();
    DHT22_Init();
    Relay_Init();
    Servo_Init();

    xQueue = xQueueCreate(3, sizeof(Data_t));
    vSemaphoreCreateBinary(xBinarySemaphore);

    xTaskCreate(SensorTask, "SensorTask", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vSenderTask, "vSenderTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vReceiverTask, "vReceiverTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1);
}

void UARTIntHandler(void) {
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, ui32Status);

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    while(UARTCharsAvail(UART1_BASE)) {
        uint8_t character = UARTCharGetNonBlocking(UART1_BASE);
        buffer[index] = character;
        index++;

        if ((character == 10 || character == 13)) {
            buffer[index - 1] = 10;
            xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
        }

        if (index >= BUFFER_SIZE) index = 0;
    }

    if( xHigherPriorityTaskWoken == pdTRUE ) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void SensorTask(void *pvParameters) {
    DHT22_Data_t DHT22_Data;
    char tempBuffer[DHT22_BUFFER_SIZE];
    char humiBuffer[DHT22_BUFFER_SIZE];
    char *tempStr;
    char *humiStr;

    while(1) {
        portTickType    xLastWakeTime;
        xLastWakeTime = xTaskGetTickCount();
        DHT22_State_t state = DHT22_Read_Data(&DHT22_Data);

        tempStr = DHT22_To_String(DHT22_Data.temperature, tempBuffer);
        humiStr = DHT22_To_String(DHT22_Data.humidity, humiBuffer);

        if (state == VALID) {
            UARTprintf("Temperature: %s\n", tempStr);
            UARTprintf("Humidity: %s\n\n", humiStr);
        }
        else if (state == INVALID) {
            UARTprintf("Data is invalid\n");
        }
        else {
            UARTprintf("No response from DHT22\n");
        }

        // Transmit data to ESP32-Cam
        char *tempPtr = tempStr;
        char *humiPtr = humiStr;
        while (*tempPtr != '\0') {
            UARTCharPutNonBlocking(UART1_BASE, *tempPtr);
            tempPtr++;
        }
        UARTCharPutNonBlocking(UART1_BASE, ',');
        while (*humiPtr != '\0') {
            UARTCharPutNonBlocking(UART1_BASE, *humiPtr);
            humiPtr++;
        }
        UARTCharPutNonBlocking(UART1_BASE, '\0');

        vTaskDelayUntil(&xLastWakeTime, 2000 / portTICK_RATE_MS);
    }
}

static void vSenderTask(void *pvParameters) {
    Data_t data;
    int i;

    while(1) {
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS) {
            for (i = 2; i < index; i++) {
                if (buffer[i] == 10) {
                    if (buffer[i  - 1] != 10 && buffer[i - 2] != 10) {
                        data.type = buffer[i - 2];
                        data.value = buffer[i - 1];
                    }
                }
            }

            index = 0;

            if (xQueueSend(xQueue, &data, portMAX_DELAY) == pdPASS) {
                UARTprintf("SenderTask: write queue success\n");
            }
            else {
                UARTprintf("SenderTasker: write queue error\n");
            }
        }
    }
}

static void vReceiverTask(void *pvParameters) {
    Data_t   data;
    portBASE_TYPE xStatus;

    while(1) {
        UARTprintf( "ReceiverTask-trying to read queue\n" );
        xStatus = xQueueReceive( xQueue, &data, portMAX_DELAY );
        if (xStatus == pdPASS) {
            if (data.type == 'R') {
                if (data.value == '0') {
                    Relay_Control(ON);
                    UARTprintf("ReceiverTask: Relay ON\n");
                }
                else if (data.value == '1') {
                    Relay_Control(OFF);
                    UARTprintf("ReceiverTask: Relay OFF\n");
                }
                else {
                    UARTprintf("ReceiverTask: Relay Undefined\n");
                }
            }
            else if (data.type == 'S') {
                if (data.value == '0') {
                    Servo_Control(0);
                    UARTprintf("ReceiverTask: Servo LEFT\n");
                }
                else if (data.value == '1') {
                    Servo_Control(90);
                    UARTprintf("ReceiverTask: Servo CENTER\n");
                }
                else if (data.value == '2') {
                    Servo_Control(180);
                    UARTprintf("ReceiverTask: Servo RIGHT\n");
                }
                else {
                    UARTprintf("ReceiverTask: Servo Undefined\n");
                }
            }
            else {
                UARTprintf("ReceiverTask: data.type is not correct\n");
            }
        }
    }
}
