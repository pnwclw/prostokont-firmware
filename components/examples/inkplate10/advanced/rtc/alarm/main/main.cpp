/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC time and alarm example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to use the PCF85063 real-time clock (RTC)
 *              integrated on the Inkplate 10 board. The example shows how
 *              to set time and date, configure an alarm, read current time,
 *              and display it on the e-paper screen using partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Build and flash to Inkplate 10.
 * 2) If RTC is not set, initialize time and date in the code.
 * 3) The program configures an RTC alarm.
 * 4) Current time is periodically read and displayed on the screen.
 *
 * Expected output:
 * - Inkplate display shows current date and time.
 * - Alarm event can be detected and handled in the sketch.
 *
 * Notes:
 * - Inkplate 10 uses the PCF85063 RTC chip.
 * - Partial update works only in 1-bit (black & white) mode.
 * - It is not recommended to use partial update on the first refresh after
 * power-up.
 * - Perform a full refresh every 5–10 partial updates to maintain display
 * quality.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_ALARM";

#define REFRESH_DELAY_MS 1000

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
static uint8_t s_alarmMinutes = 50;
static uint8_t s_alarmSeconds = 40;
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

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(5);
  display.setTextColor(BLACK, WHITE);

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
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

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
    display.setCursor(100, 300);
    printTime(display, hour, minutes, seconds, day, weekday, month, year);

    if (display.rtc.checkAlarmFlag()) {
      display.rtc.clearAlarmFlag();
      display.setCursor(400, 400);
      display.print("ALARM!");
      ESP_LOGI(TAG, "Alarm triggered!");
    }

    if (n > 9) {
      display.display(); // full refresh
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }
  }
}