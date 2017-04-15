#include "ui.h"

void uiInit(Adafruit_SSD1306 *display) {
  display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  uiRenderStatus(display, "Init");
}

void uiPrintNumberPadded(Adafruit_SSD1306 *display, unsigned long number, byte width) {
  int currentMax = 10;
  for (byte i = 1; i < width; i++) {
    if (number < (unsigned long)currentMax) {
      display->print("0");
    }
    currentMax *= 10;
  }
  display->print(number);
}

void uiRenderHeader(Adafruit_SSD1306 *display) {
  display->setTextSize(2);
  display->setTextColor(WHITE);
  display->setCursor(128 / 2 - 8 * 12 / 2, 5);
  display->print("ISTROBOT");

  display->drawFastHLine(0, 23, 128, WHITE);
  display->drawFastHLine(0, 24, 128, WHITE);
}

void uiRenderStatus(Adafruit_SSD1306 *display, char const *status) {
  display->clearDisplay();
  uiRenderHeader(display);
  display->setCursor(128 / 2 - strlen(status) * 12 / 2, 48);
  display->print(status);
  display->display();
}

void uiRenderCalibration(Adafruit_SSD1306 *display, int value, int threshold) {
  display->clearDisplay();
  uiRenderHeader(display);
  display->setCursor(1 * 12, 48);
  display->print(value > threshold ? "OK" : "CALIB");
  display->setCursor((12 - 5) * 12, 48);
  display->print(value);
  display->display();
}

void uiReportTime(Adafruit_SSD1306 *display, unsigned long timeDiff) {
  int minutes = timeDiff / minute;
  int seconds = (timeDiff % minute) / second;
  int milliseconds = ((timeDiff % minute) % second);

  display->clearDisplay();

  uiRenderHeader(display);

  display->setCursor(128 / 2 - 9 * 12 / 2, 30);
  uiPrintNumberPadded(display, minutes, 2);
  display->print(":");
  uiPrintNumberPadded(display, seconds, 2);
  display->print(".");
  uiPrintNumberPadded(display, milliseconds, 3);

  int digits = 0;
  unsigned long n = timeDiff;
  while(n != 0) {
      n /= 10;
      digits++;
  }

  display->setCursor(128 / 2 - digits * 12 / 2, 48);
  display->print(timeDiff);

  display->display();
}
