#include "config.h"

int configReadValue(int address) {
  uint8_t checkValue = EEPROM.read(address);
  if (checkValue == 255)
    return (int)EEPROM.read(address + 1);
  else
    return -1;
}

void configWriteValue(int address, uint8_t value) {
  EEPROM.write(address, 255);
  EEPROM.write(address + 1, value);
}

int configReadIgnoreTime() {
  return configReadValue(EEPROM_IGNORE_TIME_ADDRESS);
}

void configWriteIgnoreTime(uint8_t value) {
  configWriteValue(EEPROM_IGNORE_TIME_ADDRESS, value);
}
