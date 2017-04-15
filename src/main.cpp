#include <main.h>

void reloadConfig() {
  ignoreTimeValue = (uint8_t)(configReadIgnoreTime() == -1 ? 1 : configReadIgnoreTime());
}

void setup() {
  // Read EEPROM config
  reloadConfig();

  // Set pins
  pinMode(LIGHT_OUTPUT_PIN, OUTPUT);
  pinMode(REPORT_PIN1, OUTPUT);
  pinMode(REPORT_PIN2, OUTPUT);
  pinMode(BUTTON_INPUT_PIN, INPUT);

  // Init display
  uiInit(&display);

  // Init serial
  cmdLine.reserve(128);
  Serial.begin(19200);
}

void reportDigital(int state) {
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
  commSend("R");
  reportDigital(LOW);

  uiRenderStatus(&display, "READY");
}

void processSerialCommands() {
  if (cmdComplete) {
    if (cmdLine[0] == 'R') {
      stopwatchReset();
    } else if (cmdLine[0] == '0') {
      if (cmdLine.length() == 3) {
        ignoreTimeValue = (byte)(cmdLine[2] - '0');
        configWriteIgnoreTime(ignoreTimeValue);
        reloadConfig();
      }

      commSendWithULongParam("0", (unsigned long)ignoreTimeValue);
    }

    // Clear the cmdLine
    cmdLine = "";
    cmdComplete = false;
  }
}

void checkResetButton() {
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
}

void loop() {
  processSerialCommands();
  checkResetButton();

  // Light reading
  valOff = readLaserOff();
  valOn = readLaserOn();
  valDiff = valOn - valOff;

  if (stopwatchState == STOPWATCH_DEBUG) {
    commSendCalibrationValue(valDiff);
    uiRenderCalibration(&display, (int)valDiff, LIGHT_CALIBRATION_THRESHOLD);
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
      commSend("S");
      reportDigital(HIGH);
      uiRenderStatus(&display, "RUNNING...");
      return;
    }
    if (stopwatchState == STOPWATCH_RUNNING) {
      timeFinish = millis();
      unsigned long timeDiff = timeFinish - timeStart;
      stopwatchState = STOPWATCH_FINISHED;
      commSendWithULongParam("F", timeDiff);
      reportDigital(LOW);
      uiReportTime(&display, timeDiff);
      return;
    }
  }

}
