/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display current time and date fetched from the Internet (NTP)
 *              and refresh periodically.
 *
 * @details     This example demonstrates how to obtain the current time from an
 *              NTP server over WiFi and display it on the Inkplate 2 e-paper
 *              display. The sketch connects to WiFi, syncs time via NTP, then
 *              enters a loop where it reads the current time, formats hours/
 *              minutes and the calendar date, and performs a full display
 *              refresh every DELAY_TIME_MS milliseconds.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi connection + Internet access (NTP)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 * - WiFi:       configure ssid/pass in your WiFi component
 * - Timezone:   set TIME_ZONE (hours offset from UTC)
 * - Update period: set DELAY_TIME_MS (ms)
 *
 * How to use:
 * 1) Set your local timezone offset in TIME_ZONE.
 * 2) Build and flash to Inkplate 2.
 * 3) The device connects to WiFi, syncs time via NTP, and displays
 *    the current time and date.
 * 4) The screen refreshes every DELAY_TIME_MS milliseconds.
 *
 * Expected output:
 * - Display: large HH:MM time, and DD.MM.YYYY date below it.
 *
 * Notes:
 * - Display mode is 1-bit (BW). This sketch uses full refresh (display()).
 * - Frequent full refreshes increase update time and may cause more visible
 *   flashing; consider longer intervals for battery-powered use.
 * - WiFi must remain connected for each NTP fetch. If WiFi is unavailable,
 *   time retrieval will fail; add error handling for production use.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "WiFi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

#define DELAY_TIME_MS (40 * 1000)
#define TIME_ZONE 2 // UTC+2, adjust to your timezone

static const char *TAG = "RTC_ALARM";

static void getTime(struct tm *t, int timeZone) {
  time_t nowSecs = time(nullptr) + timeZone * 3600ULL;
  gmtime_r(&nowSecs, t);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextColor(INKPLATE2_BLACK);

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  // Sync time via NTP — your WiFi class handles this already
  wifi.setCurrentTime();

  struct tm currentTime;

  while (true) {
    getTime(&currentTime, TIME_ZONE);

    display.clearDisplay();

    display.setCursor(0, 10);
    display.setTextSize(7);
    display.printf("%2.1d:%02d\n", currentTime.tm_hour, currentTime.tm_min);

    display.setTextSize(3);
    display.printf(" %2.1d.%2.1d.%04d\n", currentTime.tm_mday,
                   currentTime.tm_mon + 1, currentTime.tm_year + 1900);

    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_TIME_MS));
  }
}