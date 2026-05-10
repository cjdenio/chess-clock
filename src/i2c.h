#include <stdint.h>

void i2c_init();
int i2c_write(uint8_t addr, const uint8_t *data, uint8_t len);
