#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 display(3);

static const long minute = 60000; // 60000 milliseconds in a minute
static const long second =  1000; // 1000 milliseconds in a second

void uiInit(Adafruit_SSD1306 *display);
void uiRenderHeader(Adafruit_SSD1306 *display);
void uiRenderStatus(Adafruit_SSD1306 *display, char const *status);
void uiRenderCalibration(Adafruit_SSD1306 *display, int value, int threshold);
void uiPrintNumberPadded(Adafruit_SSD1306 *display, unsigned long number, byte width);
void uiReportTime(Adafruit_SSD1306 *display, unsigned long timeDiff);
