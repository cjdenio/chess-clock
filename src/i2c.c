#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "i2c.h"

void i2c_init() {
  DDRA |= _BV(SDA_BIT) | _BV(SCL_BIT);  // Set SDA and SCL to output
  PORTA |= _BV(SDA_BIT) | _BV(SCL_BIT); // Write HIGH to SDA and SCL
}

#define HIGH 1
#define LOW 0

void port_a_put(uint8_t pin, bool value) {
  if (value) {
    PORTA |= _BV(pin);
    DDRA &= ~_BV(pin);
  } else {
    PORTA &= ~_BV(pin);
    DDRA |= _BV(pin);
  }
}

int i2c_write_byte(uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    port_a_put(SDA_BIT, (byte & 0x80) != 0);
    byte <<= 1;

    // pulse clock
    port_a_put(SCL_BIT, HIGH);
    port_a_put(SCL_BIT, LOW);
  }

  port_a_put(SDA_BIT, HIGH);

  // check ACK

  port_a_put(SCL_BIT, HIGH);

  int ack = PINA & _BV(SDA_BIT);

  port_a_put(SCL_BIT, LOW);

  return ack;
}

int i2c_write(uint8_t addr, const uint8_t *data, uint8_t len) {
  // START condition
  port_a_put(SDA_BIT, LOW);

  port_a_put(SCL_BIT, LOW);

  // write address
  int ack = i2c_write_byte(addr);
  if (ack) {
    port_a_put(SDA_BIT, HIGH);
    port_a_put(SCL_BIT, HIGH);
    return 1;
  }

  for (int i = 0; i < len; i++) {
    int ack = i2c_write_byte(data[i]);
    if (ack) {
      port_a_put(SDA_BIT, HIGH);
      port_a_put(SCL_BIT, HIGH);
      return 1;
    }
  }

  // STOP condition
  port_a_put(SCL_BIT, HIGH);
  port_a_put(SDA_BIT, HIGH);

  return 0;
}
