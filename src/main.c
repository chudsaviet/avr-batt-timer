#define F_CPU 1000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Voltages units are determined based on the fact MCU supply is 5V and ADC
// measures in 0-255 units where 255 is the supply voltage. We also have 1/3
// voltage divider in the circuit.
#define VOLTAGE_UNITS_ALTERNATOR_ON 225  // ~ 13.2V
#define VOLTAGE_UNITS_BATTRY_LOW 211     // ~ 12.4V

// If an iteration happens every 4 seconds, 225 iterations is approximately 15 minutes.
#define LOAD_OFF_TIMEOUT_ITERATIONS_COUNT 225

typedef enum MachineState {
  MACHINE_STATE_ALTERNATOR_ON,
  MACHINE_STATE_WAITING,
  MACHINE_STATE_LOAD_OFF
} MachineState;

typedef enum VoltageState {
  VOLTAGE_STATE_ALTERNATOR_ON,
  VOLTAGE_STATE_BATTERY_OK,
  VOLTAGE_STATE_BATTERY_LOW
} VoltageState;

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

  // Setup watchdog.
  // Set to 2s timeout, enable watchdog interrupt, disable watchdog reset.
  WDTCSR = (0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0) | (1 << WDIE) |
           (0 << WDE);

  // Enable interrupts back.
  sei();
}

uint8_t read_voltage_units() {
  // Enable and power on the ADC. Enable interrupt.
  ADCSRA |= (1 << ADEN) | (1 << ADIE);
  PRR |= 1 << PRADC;

  // Go to ADC Noise Reduction sleep mode.
  // ADC conversion will start.
  SMCR = SLEEP_MODE_ADC | (1 << SE);
  sleep_cpu();

  // Woke up by the ADC interrupt when converstion has been completed.
  return ADCL;
}

VoltageState get_voltage_state() {
  uint8_t voltage_units = read_voltage_units();
  if (voltage_units <= VOLTAGE_UNITS_BATTRY_LOW) {
    return VOLTAGE_STATE_BATTERY_LOW;
  } else if (voltage_units <= VOLTAGE_UNITS_ALTERNATOR_ON) {
    return VOLTAGE_STATE_BATTERY_OK;
  } else {
    return VOLTAGE_STATE_ALTERNATOR_ON;
  }
}

int main() {
  setup();

  MachineState machine_state = set_machine_state(MACHINE_STATE_ALTERNATOR_ON);

  while (true) {
    VoltageState voltage_state = get_voltage_state();

    // Power down and wait to be woke up by a watchdog interrupt.
    // Disable and power down ADC and timer.
    ADCSRA &= ~(1 << ADEN);
    PRR = (1 << PRADC) | (1 << PRTIM0);
    SMCR = SLEEP_MODE_PWR_DOWN | (1 << SE);
    sleep_cpu();
  }
  return 0;
}