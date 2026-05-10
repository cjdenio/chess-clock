#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "as1115.h"
#include "i2c.h"

#if MAIN_TIME > 5999000
#error Main time too large.
#endif

#define PLAYER_A_BUTTON 0
#define PLAYER_B_BUTTON 1

volatile uint32_t ms_since_boot = 0;

typedef enum {
	STOPPED,
	PLAYER_A,
	PLAYER_B
} state_type;

volatile state_type state = STOPPED;

volatile uint32_t player_a_timer = MAIN_TIME;
volatile uint32_t player_b_timer = MAIN_TIME;
volatile uint32_t move_started_at = MAIN_TIME;

volatile uint32_t finished_at = 0;

ISR(TIM1_COMPA_vect)
{
	ms_since_boot++;

	if(state == PLAYER_A) {
		player_a_timer--;
		if(player_a_timer == 0) { state = STOPPED; finished_at = ms_since_boot; }
	} else if(state == PLAYER_B) {
		player_b_timer--;
		if(player_b_timer == 0) { state = STOPPED; finished_at = ms_since_boot; }
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

#define DISPLAY_LEFT 0
#define DISPLAY_RIGHT 1

void render_timer(unsigned long ms, uint8_t display) {
	uint8_t digits[4] = {0xFF, 0xFF, 0xFF, 0xFF};

	if (ms == 0) {
		memset(&digits, 0x0A, 4); // show all dashes
		goto render;
	}

	if (ms >= 60000) {
		uint8_t sec = ms / 1000 % 60;
		uint8_t min = ms / 1000 / 60;

		digits[3] = sec % 10;
		digits[2] = sec / 10;

		if (min != 0) {
			digits[1] = min % 10;
			if (min >= 10) {
				digits[0] = min / 10;
			}
		}
	} else {
		uint8_t cs = ms / 10 % 100;
		uint8_t sec = ms / 1000;
		digits[3] = cs % 10;
		digits[2] = cs / 10;
		if (sec != 0) {
			digits[1] = sec % 10;
			if (sec >= 10) {
				digits[0] = sec / 10;
			}
		}
	}

render:
	switch(display) {
		case DISPLAY_LEFT:
			as1115_send_command(0x01, digits[0]);
		    as1115_send_command(0x02, digits[1]);
		    as1115_send_command(0x03, digits[2]);
		    as1115_send_command(0x04, digits[3]);
			break;
		case DISPLAY_RIGHT:
			as1115_send_command(0x05, digits[0]);
		    as1115_send_command(0x06, digits[1]);
		    as1115_send_command(0x07, digits[2]);
		    as1115_send_command(0x08, digits[3]);
			break;
	}
}

uint32_t get_increment(uint32_t player_timer) {
	if (state == STOPPED) {
		return 0;
	}

	#ifdef TIME_INCREMENT
		#if INCREMENT_MODE == MODE_FISCHER
			return TIME_INCREMENT;
		#elif INCREMENT_MODE == MODE_BRONSTEIN
			uint32_t difference = move_started_at - player_timer;
			if(difference < TIME_INCREMENT) {
				return difference;
			} else {
				return TIME_INCREMENT;
			}
		#endif
	#else
	return 0;
	#endif
}

void reset() {
	cli();
	state = STOPPED;
	player_a_timer = MAIN_TIME;
	player_b_timer = MAIN_TIME;
	move_started_at = MAIN_TIME;
	finished_at = 0;
	sei();
	render_timer(player_a_timer, DISPLAY_LEFT);
	render_timer(player_b_timer, DISPLAY_RIGHT);
}

int main() {
	cli();
	setupio();
	i2c_init();
	setup_clock();
	sei();

	render_timer(MAIN_TIME, DISPLAY_LEFT);
	render_timer(MAIN_TIME, DISPLAY_RIGHT);

    as1115_send_command(DECODE_MODE_REG, 0xFF); // decode
    as1115_send_command(SCAN_LIMIT_REG, 0x07); // enable all digits
    as1115_send_command(GLOBAL_INTENSITY_REG, 0x06); // set brightness
    as1115_send_command(SHUTDOWN_REG, 0x01); // turn on

	while(true) {
		render_timer(player_a_timer, DISPLAY_LEFT);
		render_timer(player_b_timer, DISPLAY_RIGHT);

		cli();
		if(button_pressed(PLAYER_A_BUTTON) && state != PLAYER_B && player_b_timer != 0 && player_a_timer != 0) {
			player_a_timer += get_increment(player_a_timer);
			state = PLAYER_B;
			move_started_at = player_b_timer;
		}

		if(button_pressed(PLAYER_B_BUTTON) && state != PLAYER_A && player_b_timer != 0 && player_a_timer != 0) {
			player_b_timer += get_increment(player_b_timer);
			state = PLAYER_A;
			move_started_at = player_a_timer;
		}
		sei();

		if (state == STOPPED && (player_a_timer == 0 || player_b_timer == 0) && (ms_since_boot > finished_at + RESET_DELAY)) {
			reset();
		}
	}
}
