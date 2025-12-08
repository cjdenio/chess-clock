#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"

#define PLAYER_A_BUTTON 0
#define PLAYER_B_BUTTON 1

uint32_t ms_since_boot = 0;

typedef enum {
	STOPPED,
	PLAYER_A,
	PLAYER_B
} state_type;

state_type state = STOPPED;

uint32_t player_a_timer = STARTING_TIME_MS;
uint32_t player_b_timer = STARTING_TIME_MS;

ISR(TIM1_COMPA_vect)
{
	ms_since_boot++;

	if(state == PLAYER_A) {
		player_a_timer--;
		if(player_a_timer == 0) { state = STOPPED; }
	} else if(state == PLAYER_B) {
		player_b_timer--;
		if(player_b_timer == 0) { state = STOPPED; }
	}
}

void setupio(void) {
	// Set port A to inputs
	DDRA = 0x00;
	PORTA |= _BV(PORTA0) | _BV(PORTA1); // pull-up

	// Set port B to output
	DDRB = 0xFF;
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
		cli();
		if(button_pressed(PLAYER_A_BUTTON) && player_b_timer != 0) {
			state = PLAYER_B;
		}

		if(button_pressed(PLAYER_B_BUTTON) && player_a_timer != 0) {
			state = PLAYER_A;
		}

		PORTB = 1 << state;
		sei();
	}
}

