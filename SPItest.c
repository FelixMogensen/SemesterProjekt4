/*
 * stdint and stdbool provides integer and bool data types
 * hw_memmap contains definitions for the memory map of the microcontroller, such as addresses for peripherals
 * sysctl provides functions to control system settings like clock configuration and enabling peripherals
 * gpio provides functions for controlling the GPIO pins
 * ssi provides functions to configure the SSI (SPI)
 * pinmap defines macros that map certain pins to alternate functions
 * 
 * note that the PA4 (RX) and PA5 (TX) pins must be physically connected for this SPI test to function
 */

#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "sysctl.h"
#include "gpio.h"
#include "ssi.h"
#include "pin_map.h"

int main(void)
{
    // Clock set to 16 MHz from the internal crystal oscillator
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Enabling peripherals for SPI and LED, GPIOF used for LED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // Configure LED pin (PF1)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // Configure and enable GPIO pins for SSI0 (SPI)
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    // Configure SSI0 as SPI master mode, and set to 1Mhz, 8 bit data
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    // Enabling the SSI module
    SSIEnable(SSI0_BASE);

    uint32_t dataToSend = 0x55; // Arbitrary test byte
    uint32_t receivedData = 0;

    // Loop forever
    while (1)
    {
        // Send data over SPI
        SSIDataPut(SSI0_BASE, dataToSend);

        // Wait for transmission to complete
        while(SSIBusy(SSI0_BASE));

        // Read received data
        SSIDataGet(SSI0_BASE, &receivedData);

        // Simple verification of data: if loopback test functions, receivedData should equal dataToSend
        if(receivedData == dataToSend)
        {
            // Toggle LED to indicate success
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            SysCtlDelay(SysCtlClockGet()/3); // Delay 0.5 sec
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        }

        SysCtlDelay(SysCtlClockGet()/3); // 0.5 sec delay with 16 MHz clock
    }
}
