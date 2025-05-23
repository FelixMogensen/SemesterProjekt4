#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "pwm.h"

// A helper function to safely compute partial duty
static uint32_t computePulseWidth(uint32_t loadVal, uint8_t percent)
{
    // 'loadVal' is the final value used in PWMGenPeriodSet(...),
    // so the period is (loadVal + 1) timer counts.
    //
    // If we want, say, 25% duty:
    //  pulseWidth = (loadVal+1) * 25/100
    // We clamp it so it never exceeds 'loadVal' (the maximum valid).
    //
    uint32_t result = ((loadVal + 1) * percent) / 100;
    if(result > loadVal)
        result = loadVal;
    return result;
}

int main(void)
{
    // -----------------------------------------------------------------------
    // 1) Configure system clock to 16 MHz
    // -----------------------------------------------------------------------
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // -----------------------------------------------------------------------
    // 2) LED for debugging on PF1 (optional)
    // -----------------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // -----------------------------------------------------------------------
    // 3) Enable L6203 "Enable" pin on PA3
    // -----------------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
    // Drive PA3 high => motor driver enabled
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);

    // -----------------------------------------------------------------------
    // 4) Set up PWM on PB6 (M0PWM0) and PB7 (M0PWM1)
    // -----------------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {}

    // Configure PB6 -> M0PWM0
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    // Configure PB7 -> M0PWM1
    GPIOPinConfigure(GPIO_PB7_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    // -----------------------------------------------------------------------
    // 5) Set the PWM clock => /64
    //    If the main clock is 16 MHz, then PWM runs at 16MHz/64 = 250 kHz
    // -----------------------------------------------------------------------
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    // -----------------------------------------------------------------------
    // 6) Configure a 1 kHz PWM frequency
    // -----------------------------------------------------------------------
    uint32_t pwmFreq  = 1000; // 1 kHz
    uint32_t pwmClock = SysCtlClockGet() / 64; // e.g. 16MHz/64=250 kHz
    uint32_t loadVal  = (pwmClock / pwmFreq) - 1; // e.g. 249

    // Generator 0 (M0PWM0 & M0PWM1) in down-count mode
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, loadVal);

    // -----------------------------------------------------------------------
    // 7) Choose your duty cycle (percentage)
    // -----------------------------------------------------------------------
    uint8_t dutyPct = 40;  // e.g. 25%
    uint32_t pulseWidth = computePulseWidth(loadVal, dutyPct);

    // Sign-magnitude approach (Forward):
    // - PB6 (M0PWM0) has the partial duty
    // - PB7 (M0PWM1) is 0 => we do not drive it
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0); // PB6 => e.g. 25%
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pulseWidth);          // PB7 => 0%

    // Enable outputs for M0PWM0 and M0PWM1
    PWMOutputState(PWM0_BASE, (PWM_OUT_0_BIT | PWM_OUT_1_BIT), true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    // -----------------------------------------------------------------------
    // 8) Main loop: blink PF1 LED so we know code is running
    // -----------------------------------------------------------------------
    while(1)
    {
        // LED on
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(SysCtlClockGet()/3);

        // LED off
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        SysCtlDelay(SysCtlClockGet()/3);
    }

    return 0;
}
