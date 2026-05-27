/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm display example for Soldered Inkplate 6Color.
 *
 * @details     Sets the on-board RTC to a configured time and date, then sets
 *              a matching alarm. The current time is read from the RTC every
 *              minute and displayed on screen. When the alarm time is reached
 *              the word "ALARM!" is shown and the alarm flag is cleared.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Adjust the s_hour/s_minutes/... and alarm variables at the top of the
 * file. 2) Build and flash to Inkplate 6Color. 3) The display shows the running
 * time; "ALARM!" appears when the alarm fires.
 *
 * Expected output:
 * - Current time and date shown in green, updating every minute.
 * - "ALARM!" printed when the alarm time is reached.
 *
 * Notes:
 * - The alarm triggers only once; clear and re-set to repeat.
 * - Time is refreshed every REFRESH_DELAY_MS milliseconds (default 60000 ms).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6Color in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_ALARM";

#define REFRESH_DELAY_MS 60000

// Set clock
static uint8_t s_hour = 12;
static uint8_t s_minutes = 50;
static uint8_t s_seconds = 30;

// Set date (tm_wday: 0=Sunday, 1=Monday, ...)
static uint8_t s_weekday = 1;
static uint8_t s_day = 20;
static uint8_t s_month = 2;    // tm_mon is 0-based, handled below
static uint16_t s_year = 2026; // years since 1900, handled below

// Alarm (10 seconds after start)
static uint8_t s_alarmHour = 12;
static uint8_t s_alarmMinutes = 51;
static uint8_t s_alarmSeconds = 30;
static uint8_t s_alarmWeekday = 1;
static uint8_t s_alarmDay = 20;

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

static void printTime(Inkplate &display, uint8_t hour, uint8_t minutes,
                      uint8_t seconds, uint8_t day, uint8_t weekday,
                      uint8_t month, uint16_t year) {
  const char *wday[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};

  print2Digits(display, hour);
  display.print(':');
  print2Digits(display, minutes);
  display.print(':');
  print2Digits(display, seconds);
  display.print(' ');
  display.print(wday[weekday % 7]);
  display.print(", ");
  print2Digits(display, day);
  display.print('/');
  print2Digits(display, month);
  display.print('/');
  display.print(year);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setTextSize(3);
  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);

  // Build tm struct for setTime()
  tm t = {};
  t.tm_hour = s_hour;
  t.tm_min = s_minutes;
  t.tm_sec = s_seconds;
  t.tm_wday = s_weekday;
  t.tm_mday = s_day;
  t.tm_mon = s_month;
  t.tm_year = s_year;

  esp_err_t err = display.rtc.setTime(t);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  err = display.rtc.setAlarm(s_alarmSeconds, s_alarmMinutes, s_alarmHour,
                             s_alarmDay, s_alarmWeekday);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setAlarm failed: %s", esp_err_to_name(err));

  int n = 0;

  while (true) {

    // Individual getters call updateTime() internally each time,
    // so call getTime() once and unpack to avoid 7 separate I2C reads
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    uint8_t hour = (uint8_t)now.tm_hour;
    uint8_t minutes = (uint8_t)now.tm_min;
    uint8_t seconds = (uint8_t)now.tm_sec;
    uint8_t day = (uint8_t)now.tm_mday;
    uint8_t weekday = (uint8_t)now.tm_wday;
    uint8_t month = (uint8_t)(now.tm_mon);   // back to 1-based for display
    uint16_t year = (uint16_t)(now.tm_year); // back to 2-digit year for display

    display.clearDisplay();
    display.setCursor(50, 100); // Set position of the text
    display.setTextColor(INKPLATE_GREEN,
                         INKPLATE_WHITE); // Set text color and background
    printTime(display, hour, minutes, seconds, day, weekday, month, year);

    if (display.rtc.checkAlarmFlag()) {
      display.rtc.clearAlarmFlag();
      display.setCursor(140, 140);
      display.print("ALARM!");
      ESP_LOGI(TAG, "Alarm triggered!");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}