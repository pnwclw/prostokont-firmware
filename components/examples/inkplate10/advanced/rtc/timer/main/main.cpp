/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC timer functionality example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to use the PCF85063A real-time clock (RTC)
 *              timer functionality on the Inkplate 10 board. The example shows
 *              how to set time and date, configure the RTC timer, read current
 *              time values, and display them on the e-paper screen using
 *              partial updates.
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
 * 2) Initialize RTC time and date if not already configured.
 * 3) Configure the RTC timer in the code.
 * 4) The timer event is handled while current time is read and displayed.
 *
 * Expected output:
 * - Inkplate display shows the current date and time.
 * - Timer functionality operates according to configured interval.
 *
 * Notes:
 * - Inkplate 10 uses the PCF85063A RTC chip.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_TIMER";

#define REFRESH_DELAY_MS 700

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
static const uint8_t COUNTDOWN_TIME = 15;

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

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(5);
  display.setTextColor(BLACK, WHITE);

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
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    display.clearDisplay();
    display.setCursor(100, 300);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday, now.tm_mon, now.tm_year);

    if (n > 9) {
      display.display();
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }
  }
}