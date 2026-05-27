/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple RTC clock display for Soldered Inkplate 6Color.
 *
 * @details     Sets the on-board RTC to a configured time and date, then
 *              continuously reads and displays the current time on the
 *              e-paper screen. The display is refreshed every minute.
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
 * 1) Set the desired starting time and date via the s_hour/s_minutes/...
 * variables. 2) Build and flash to Inkplate 6Color. 3) The display shows the
 * current time, refreshing every minute.
 *
 * Expected output:
 * - Current time and date shown in black, updating every 60 seconds.
 *
 * Notes:
 * - The RTC is set once at startup; time persists while the board has power.
 * - Adjust REFRESH_DELAY_MS for a different update interval.
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

static const char *TAG = "RTC_SIMPLE";

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

  display.clearDisplay(); // Clear frame buffer of display
  display.setTextSize(
      3); // Set text to be 3 times bigger than classic 5x7 px text
  display.setTextColor(INKPLATE_BLACK,
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