#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "hw_types.h"
#include "sysctl.h"
#include "gpio.h"
#include "ssi.h"
#include "pin_map.h"

int main(void)
{
    /* ------------------------------------------------------------------
     *  1.  Clocks
     * ------------------------------------------------------------------ */
    SysCtlClockSet(SYSCTL_SYSDIV_1 |
                   SYSCTL_USE_OSC  |
                   SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);           // 16 MHz

    /* ------------------------------------------------------------------
     *  2.  Peripherals
     * ------------------------------------------------------------------ */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)) {}

    /* PF1 – on-board red LED */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    /* SPI pin-mux */
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);   // SCLK
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);   // CS_n
    GPIOPinConfigure(GPIO_PA5_SSI0TX);    // MOSI
    /* PA4 (MISO) left floating or pulled down – we don't use it */
    GPIOPinTypeSSI(GPIO_PORTA_BASE,
                   GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5);

    /* ------------------------------------------------------------------
     *  3.  SSI0: Master, Mode-0, 1 MHz, 8-bit
     * ------------------------------------------------------------------ */
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(),
                       SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER,
                       1000000,            // 1 MHz SCK
                       8);                 // 8-bit words
    SSIEnable(SSI0_BASE);

    /* ------------------------------------------------------------------
     *  4.  Main loop – transmit 0xA5 forever
     * ------------------------------------------------------------------ */
    const uint8_t TX_BYTE = 0xA5;
    static bool   led_state = false;   // remember LED state between loops

    while (1)
    {
        SSIDataPut(SSI0_BASE, TX_BYTE);        // start transfer
        while (SSIBusy(SSI0_BASE)) {}          // wait until done

        /* Optional: drain RX so the FIFO never fills */
        uint32_t dummy;
        if (SSIDataGetNonBlocking(SSI0_BASE, &dummy)) { (void)dummy; }

        /* ---------- toggle PF1 so it stays in sync with LD0 ---------- */
        led_state = !led_state;   // flip state
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
                     led_state ? GPIO_PIN_1 : 0);

        SysCtlDelay(SysCtlClockGet() / 3);     // ~0.5 s pause (1 Hz)
    }
}
