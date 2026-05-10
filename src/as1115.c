#include "as1115.h"
#include "i2c.h"

int as1115_send_command(uint8_t cmd, uint8_t data) {
  uint8_t packet[] = {cmd, data};
  return i2c_write(0x00, packet, sizeof packet);
}
