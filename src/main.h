#include <Arduino.h>

#include <Wire.h>

#include <config.h>
#include <ui.h>
#include <comm.h>

#define REPORT_PIN1 12
#define REPORT_PIN2 13

#define LIGHT_INPUT_PIN 0
#define LIGHT_OUTPUT_PIN 5

#define BUTTON_INPUT_PIN 6

#define LIGHT_DIFF_THRESHOLD 5
#define LIGHT_CHANGE_DELAY_US 300
#define LIGHT_CALIBRATION_THRESHOLD 300

#define IGNORE_TIME_MUTIPLIER 1000

#define STOPWATCH_DEBUG -1
#define STOPWATCH_READY 0
#define STOPWATCH_IGNORING 1
#define STOPWATCH_RUNNING 2
#define STOPWATCH_FINISHED 3

static String cmdLine = "";
static bool cmdComplete = false;
static byte ignoreTimeValue = 1;
