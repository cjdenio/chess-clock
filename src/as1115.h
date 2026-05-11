#include <stdint.h>

#define DIGIT_0_REG 0x01
#define DIGIT_1_REG 0x02
#define DIGIT_2_REG 0x03
#define DIGIT_3_REG 0x04
#define DIGIT_4_REG 0x05
#define DIGIT_5_REG 0x06
#define DIGIT_6_REG 0x07
#define DIGIT_7_REG 0x08
#define DECODE_MODE_REG 0x09
#define GLOBAL_INTENSITY_REG 0x0A
#define SCAN_LIMIT_REG 0x0B
#define SHUTDOWN_REG 0x0C
#define SELF_ADDRESSING_REG 0x0D
#define FEATURE_REG 0x0E
#define DISPLAY_TEST_MODE_REG 0x0F
#define DIG_0_1_INTENSITY_REG 0x10
#define DIG_2_3_INTENSITY_REG 0x11
#define DIG_4_5_INTENSITY_REG 0x12
#define DIG_6_7_INTENSITY_REG 0x13

int as1115_send_command(uint8_t cmd, uint8_t data);
void as1115_blank_display();
