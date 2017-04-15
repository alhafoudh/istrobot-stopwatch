#include <Arduino.h>

#include <Wire.h>
#include <EEPROM.h>

#include <Adafruit_SSD1306.h>

#define REPORT_PIN1 12
#define REPORT_PIN2 13

#define LIGHT_INPUT_PIN 0
#define LIGHT_OUTPUT_PIN 5

#define BUTTON_INPUT_PIN 6

#define LIGHT_DIFF_THRESHOLD 5
#define LIGHT_CHANGE_DELAY_US 300
#define LIGHT_CALIBRATION_THRESHOLD 300

#define IGNORE_TIME_MUTIPLIER 1000
#define EEPROM_IGNORE_TIME_CHECK_ADDRESS 0
#define EEPROM_IGNORE_TIME_ADDRESS 1

#define STOPWATCH_DEBUG -1
#define STOPWATCH_READY 0
#define STOPWATCH_IGNORING 1
#define STOPWATCH_RUNNING 2
#define STOPWATCH_FINISHED 3

Adafruit_SSD1306 display(3);

String cmdLine = "";
bool cmdComplete = false;
byte ignoreTimeValue = 1;

void renderHeader() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(128 / 2 - 8 * 12 / 2, 5);
  display.print("ISTROBOT");

  display.drawFastHLine(0, 23, 128, WHITE);
  display.drawFastHLine(0, 24, 128, WHITE);
}

void renderStatus(char const *status) {
  display.clearDisplay();
  renderHeader();
  display.setCursor(128 / 2 - strlen(status) * 12 / 2, 48);
  display.print(status);
  display.display();
}

void renderCalibration(int value) {
  display.clearDisplay();
  renderHeader();
  display.setCursor(1 * 12, 48);
  display.print(value > LIGHT_CALIBRATION_THRESHOLD ? "OK" : "CALIB");
  display.setCursor((12 - 5) * 12, 48);
  display.print(value);
  display.display();
}

void readIgnoreTime() {
  int ignoreTimeMsCheck = EEPROM.read(EEPROM_IGNORE_TIME_CHECK_ADDRESS);
  if (ignoreTimeMsCheck == 255) {
    ignoreTimeValue = EEPROM.read(EEPROM_IGNORE_TIME_ADDRESS);
  }
}

void writeIgnoreTime() {
  EEPROM.write(EEPROM_IGNORE_TIME_ADDRESS, ignoreTimeValue);
  EEPROM.write(EEPROM_IGNORE_TIME_CHECK_ADDRESS, 255);
  readIgnoreTime();
}

void setup() {
  // Read EEPROM config
  readIgnoreTime();

  // Set pins
  pinMode(LIGHT_OUTPUT_PIN, OUTPUT);
  pinMode(REPORT_PIN1, OUTPUT);
  pinMode(REPORT_PIN2, OUTPUT);
  pinMode(BUTTON_INPUT_PIN, INPUT);

  // Init display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  renderStatus("Init");

  // Init serial
  cmdLine.reserve(128);
  Serial.begin(19200);
}

const long minute = 60000; // 60000 milliseconds in a minute
const long second =  1000; // 1000 milliseconds in a second

void reportTriggerDisplay(unsigned long timeDiff) {
  int minutes = timeDiff / minute;
  int seconds = (timeDiff % minute) / second;
  int milliseconds = ((timeDiff % minute) % second);

  display.clearDisplay();

  renderHeader();

  display.setCursor(128 / 2 - 9 * 12 / 2, 30);
  displayPrintNumberPadded(minutes, 2);
  display.print(":");
  displayPrintNumberPadded(seconds, 2);
  display.print(".");
  displayPrintNumberPadded(milliseconds, 3);

  int digits = 0;
  unsigned long n = timeDiff;
  while(n != 0) {
      n /= 10;
      digits++;
  }

  display.setCursor(128 / 2 - digits * 12 / 2, 48);
  display.print(timeDiff);

  display.display();
}

void displayPrintNumberPadded(unsigned long number, byte width) {
  int currentMax = 10;
  for (byte i = 1; i < width; i++) {
    if (number < (unsigned long)currentMax) {
      display.print("0");
    }
    currentMax *= 10;
  }
  display.print(number);
}

void reportTriggerSerial(char const *str) {
  Serial.print(str);
  Serial.print(" \n");
  Serial.flush();
}

void reportTriggerSerialWithTime(char const *str, unsigned long timeDiff) {
  Serial.print(str);
  Serial.print(" ");
  Serial.print(timeDiff);
  Serial.print("\n");
  Serial.flush();
}


void reportTriggerDigital(int state) {
  digitalWrite(REPORT_PIN1, state);
  digitalWrite(REPORT_PIN2, state);
}

int readLaserOff() {
  digitalWrite(LIGHT_OUTPUT_PIN, LOW);
  delayMicroseconds(LIGHT_CHANGE_DELAY_US);
  return analogRead(LIGHT_INPUT_PIN);
}

int readLaserOn() {
  digitalWrite(LIGHT_OUTPUT_PIN, HIGH);
  delayMicroseconds(LIGHT_CHANGE_DELAY_US);
  return analogRead(LIGHT_INPUT_PIN);
}

void reportCalibrationSerial(int value) {
  Serial.print("C ");
  Serial.println(value);
  Serial.flush();
}

void serialEvent() {
  while (Serial.available()) {
    char chr = (char)Serial.read();

    if (chr == '\n')
      cmdComplete = true;
    else cmdLine += chr;
  }
}

int valOff, valOn, valDiff;

int stopwatchState = STOPWATCH_DEBUG;
unsigned long timeStart;
unsigned long timeFinish;

// Reset button
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void stopwatchReset() {
  stopwatchState = STOPWATCH_READY;
  reportTriggerSerial("R");
  reportTriggerDigital(LOW);

  renderStatus("READY");
}


void loop() {
  // Commands from Serial
  if (cmdComplete) {
    if (cmdLine[0] == 'R') {
      stopwatchReset();
    } else if (cmdLine[0] == '0') {
      if (cmdLine.length() == 3) {
        ignoreTimeValue = (byte)(cmdLine[2] - '0');
        writeIgnoreTime();
      }

      Serial.print("0 ");
      Serial.println(ignoreTimeValue);
    }

    // Clear the cmdLine
    cmdLine = "";
    cmdComplete = false;
  }

  // Reset button
  int buttonReading = digitalRead(BUTTON_INPUT_PIN);

  if (buttonReading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;

      if (buttonState == HIGH) {
        stopwatchReset();
      }
    }
  }

  lastButtonState = buttonReading;

  // Light reading
  valOff = readLaserOff();
  valOn = readLaserOn();
  valDiff = valOn - valOff;

  if (stopwatchState == STOPWATCH_DEBUG) {
    reportCalibrationSerial(valDiff);
    renderCalibration((int)valDiff);
    delay(50);
    return;
  }

  if (stopwatchState == STOPWATCH_IGNORING) {
    if (millis() - timeStart > ignoreTimeValue * IGNORE_TIME_MUTIPLIER)
      stopwatchState = STOPWATCH_RUNNING;
    else
      return;
  }

  if (valDiff < LIGHT_DIFF_THRESHOLD) {
    if (stopwatchState == STOPWATCH_READY) {
      timeStart = millis();
      stopwatchState = STOPWATCH_IGNORING;
      reportTriggerSerial("S");
      reportTriggerDigital(HIGH);
      renderStatus("RUNNING...");
      return;
    }
    if (stopwatchState == STOPWATCH_RUNNING) {
      timeFinish = millis();
      unsigned long timeDiff = timeFinish - timeStart;
      stopwatchState = STOPWATCH_FINISHED;
      reportTriggerSerialWithTime("F", timeDiff);
      reportTriggerDigital(LOW);
      reportTriggerDisplay(timeDiff);
      return;
    }
  }

}
