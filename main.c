#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"

uint32_t ms_since_boot = 0;

ISR(TIM1_COMPA_vect)
{
	ms_since_boot++;
}

void setupio(void) {
	// Set port A to inputs
	DDRA = 0x00;
	PORTA |= PORTA0 | PORTA1; // pull-up

	// Set port B0 to output
	DDRB = 0x01;
}

void setup_clock(void) {
	OSCCAL = OSCILLATOR_CALIBRATION;

	TCCR1B = 0x09;
	OCR1A = 1000;
	TIMSK1 = 0x02;
}

bool button_pressed(uint8_t pin) {
	return (PINA & (1 << pin)) == 0;
}

int main() {
	setupio();
	setup_clock();
	sei();

	while(true) {
		uint32_t sec = ms_since_boot / 1000;
		if(sec % 2 == 0) {
			PORTB = 1;
		} else {
			PORTB = 0;
		}
	}
}

