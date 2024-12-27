#define F_CPU 1000000UL

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/wdt.h>

void setup() {
    // Disable all interrupts.
    cli();

    // Set CPU clock source to internal 8 MHz oscillator.
    CLKMSR = 0;
    // Divide main clock by 8, which will set clock to 1 MHz.
    CLKPSR = 0b0011;

    // Setup PORTB - all input except PORTB2.
    DDRB = 1 << PORTB2;
    // Disable internal pull-up resistor on PORTB1 and PORTB2.
    PUEB &= ~(1 << PORTB1) & ~(1 << PORTB2);

    // Set ADC muxer to connect to pin 1.
    ADMUX = ADC1;
    // Enable ADC.
    ADCSRA |= 1 << ADEN;

    // Setup watchdog.
    // Set to 2s timeout, enable watchdog interrupt, disable watchdog reset.
    WDTCSR = (0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0) | ( 1 << WDIE ) | (0 << WDE);

    // Enable interrupts back.
    sei();
}

int main()
{
    setup();

    while (1)
    {
        // Power down and wait to be woke up by a watchdog interrupt.
        // Power down ADC and timer.
        PRR = (1 << PRADC) | (1 << PRTIM0);
        SMCR = SLEEP_MODE_PWR_DOWN | (1 << SE);
        sleep_cpu();
    }
    return 0;
}