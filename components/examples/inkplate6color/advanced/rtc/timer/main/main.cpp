/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC countdown timer example for Soldered Inkplate 6Color.
 *
 * @details     Sets the on-board RTC to a configured time and starts a
 * 50-second countdown timer using the RTC hardware timer at 1 Hz. The current
 *              time is refreshed on the display every minute. When the
 * countdown reaches zero, "Timer!" is shown and the timer is disabled.
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
 * 1) Adjust COUNTDOWN_TIME and the starting time variables if desired.
 * 2) Build and flash to Inkplate 6Color.
 * 3) After COUNTDOWN_TIME seconds the display shows "Timer!".
 *
 * Expected output:
 * - Current time shown in red; "Timer!" printed when the countdown expires.
 *
 * Notes:
 * - Remove the disableTimer() call in printTime() to make the timer repeating.
 * - See the setTimer() call for available clock sources and their resolution.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_TIMER";

#define REFRESH_DELAY_MS 60000

// Set clock
static uint8_t s_hour = 12;
static uint8_t s_minutes = 50;
static uint8_t s_seconds = 30;

// Set date (weekday: 0=Sunday, 1=Monday, ...)
static uint8_t s_weekday = 4;
static uint8_t s_day = 11;
static uint8_t s_month = 11;
static uint16_t s_year = 2021;

// 15 second countdown timer
static const uint8_t COUNTDOWN_TIME = 50;

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

  if (display.rtc.checkTimerFlag()) {
    display.rtc.clearTimerFlag();
    display.rtc.disableTimer(); // remove this line to make timer repeating
    display.setCursor(400, 400);
    display.print("Timer!");
    ESP_LOGI(TAG, "Timer triggered!");
  }
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display
  display.setTextSize(
      3); // Set text to be 4 times bigger than classic 5x7 px text
  display.setTextColor(INKPLATE_RED,
                       INKPLATE_WHITE); // Set text color and background

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

  /*  RTC_TIMER_CLOCK_4096HZ  -> min = 244us,    max = 62.256ms
   *  RTC_TIMER_CLOCK_64HZ    -> min = 15.625ms, max = 3.984s
   *  RTC_TIMER_CLOCK_1HZ     -> min = 1s,       max = 255s
   *  RTC_TIMER_CLOCK_1PER60HZ-> min = 60s,      max = 4h15min
   *
   *  setTimer(clockSource, value, intEnable, intPulse)
   *  intEnable: true  = interrupt enabled
   *  intPulse:  true  = interrupt generates a pulse
   *             false = interrupt follows timer flag
   */
  err = display.rtc.setTimer(RTC_TIMER_CLOCK_1HZ, COUNTDOWN_TIME, true, false);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTimer failed: %s", esp_err_to_name(err));

  int n = 0;

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    display.clearDisplay();
    display.setCursor(10, 30);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday, now.tm_mon, now.tm_year);

    display.display();
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}