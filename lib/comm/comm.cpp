#include "comm.h"

void commSend(char const *str) {
  Serial.print(str);
  Serial.print("\n");
  Serial.flush();
}

void commSendWithULongParam(char const *str, unsigned long param) {
  Serial.print(str);
  Serial.print(" ");
  Serial.print(param);
  Serial.print("\n");
  Serial.flush();
}

void commSendCalibrationValue(int value) {
  Serial.print("C ");
  Serial.println(value);
  Serial.flush();
}
