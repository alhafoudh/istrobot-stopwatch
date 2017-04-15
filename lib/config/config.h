#include <EEPROM.h>

#define EEPROM_IGNORE_TIME_ADDRESS 0

int configReadIgnoreTime();
void configWriteIgnoreTime(uint8_t value);
