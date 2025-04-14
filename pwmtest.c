#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "sysctl.h"
#include "pin_map.h"
#include "gpio.h"
#include "pwm.h"

int main(void)
{

    // 1) System clock: 16 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // 2) Enable GPIOF for LED on PF1 (for debugging)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // 3) Enable GPIOA for EN_A (PA3) and Direction (PA4)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}

    // Configure PA3, PA4 as outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3 | GPIO_PIN_4);

    // Drive EN_A high
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);

    // Set direction: PA4 = low => forward, or high => reverse
    // let's pick forward:
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0); // 0 => forward

    // 4) Enable PWM on PB6
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {}

    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);

    // 5) Configure PWM clock/frequency
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64); // 16 MHz / 64 = 250 kHz
    uint32_t pwmFreq = 1000;
    uint32_t pwmClock = SysCtlClockGet() / 64; // 250 kHz
    uint32_t loadVal  = (pwmClock / pwmFreq) - 1; // 249

    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, loadVal);

    // 25% duty
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, loadVal / 4);

    // Enable PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    // Main loop: blink LED on PF1

    while(1)
    {
        // LED on
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(SysCtlClockGet()/3); // ~0.5 sec

        // LED off
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        SysCtlDelay(SysCtlClockGet()/3); // ~0.5 sec
    }

    return 0;
}
