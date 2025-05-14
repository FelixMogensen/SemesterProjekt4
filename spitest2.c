#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "hw_types.h"
#include "sysctl.h"
#include "gpio.h"
#include "ssi.h"
#include "pin_map.h"

// wiring (loop-back on the TM4C board)
// PA2 → W14 (SCLK)   |  PA4 ← T11 (MISO)
// PA3 → T10 (CS_n)   |  PA5 → Y14 (MOSI)
// Common GND between TM4C and FPGA

int main(void)
{
    // 16 MHz system clock
    SysCtlClockSet(SYSCTL_SYSDIV_1   |
                   SYSCTL_USE_OSC    |
                   SYSCTL_OSC_MAIN   |
                   SYSCTL_XTAL_16MHZ);

    // Enable peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)){}

    // PF1 = red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // SPI pin-mux
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);   // SCLK
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);   // CS_n
    GPIOPinConfigure(GPIO_PA4_SSI0RX);    // MISO
    GPIOPinConfigure(GPIO_PA5_SSI0TX);    // MOSI
    GPIOPinTypeSSI(GPIO_PORTA_BASE,
                   GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    // SSI0 : master | mode-0 | 1 MHz | 8-bit
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(),
                       SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
    SSIEnable(SSI0_BASE);

    // Main loop: send 0xA5 and flash LED on correct echo
    const uint8_t TX_BYTE = 0xA5;
    uint32_t      rx      = 0;

    while (1)
    {
        SSIDataPut(SSI0_BASE, TX_BYTE);        // start transfer
        while(SSIBusy(SSI0_BASE));             // wait until done
        SSIDataGet(SSI0_BASE, &rx);            // read echoed byte

        if ((uint8_t)rx == TX_BYTE)            // good echo?
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // LED on
            SysCtlDelay(SysCtlClockGet()/6);   // ~0.25 s
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);          // LED off
        }

        SysCtlDelay(SysCtlClockGet()/3);       // ~0.5 s pause
    }
}
