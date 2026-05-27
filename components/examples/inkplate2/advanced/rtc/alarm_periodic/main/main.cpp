/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       NTP-synced alarm clock on Inkplate 2 with deep sleep between
 *              wake-up checks.
 *
 * @details     This example demonstrates how to implement a simple alarm clock
 *              on Inkplate 2 using NTP-synced time and ESP32 deep sleep.
 *
 *              On each wake cycle the device connects to WiFi, fetches the
 *              current time via NTP, and checks whether the configured alarm
 *              time has been reached. If the alarm has not yet triggered, a
 *              waiting screen is shown and the ESP32 enters deep sleep for a
 *              fixed wake interval (wakeMinutes). Once the alarm time is
 *              reached or passed, an alarm screen is displayed and the device
 *              stays awake indefinitely.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi connection + Internet access (NTP)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 * - WiFi:         configure ssid/pass in your WiFi component
 * - Timezone:     set TIME_ZONE (hours offset from UTC)
 * - Alarm time:   set alarmHour, alarmMins, alarmSecs, alarmDay, alarmMon
 * - Wake interval: set wakeMinutes (how often to check the alarm)
 *
 * How to use:
 * 1) Set your alarm time and timezone.
 * 2) Build and flash to Inkplate 2.
 * 3) The device connects to WiFi, checks the alarm, and either shows the
 *    alarm screen (if time is reached) or sleeps until the next check.
 *
 * Expected output:
 * - Display: "Waiting for HH:MM on DD.MM." until the alarm time is reached,
 *   then "ALARM!" is shown and the device stays awake.
 *
 * Notes:
 * - Display mode is 1-bit (BW). Only full refresh (display()) is used.
 * - WiFi is reconnected on every wake cycle; this increases wake time and
 *   power consumption. For lower power, consider an RTC wakeup without WiFi
 *   for intermediate checks.
 * - If the alarm time is already in the past on first boot, the alarm screen
 *   is shown immediately.
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
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

static const char *TAG = "RTC_ALARM";

#define TIME_ZONE 2 // adjust to your timezone

static const int alarmHour = 14;
static const int alarmMins = 35;
static const int alarmSecs = 0;
static const int alarmDay = 27;
static const int alarmMon = 4;

// wake up every minute
static const int wakeHours = 0;
static const int wakeMinutes = 1;

static void getTime(struct tm *t, int timeZone) {
  time_t nowSecs = time(nullptr) + timeZone * 3600ULL;
  gmtime_r(&nowSecs, t);
}

static int secondsUntilAlarm(int aHour, int aMins, int aSecs, int aDay,
                             int aMon, struct tm currentTime) {
  struct tm timerTime = currentTime;
  timerTime.tm_hour = aHour;
  timerTime.tm_min = aMins;
  timerTime.tm_sec = aSecs;
  timerTime.tm_mday = aDay;
  timerTime.tm_mon = aMon - 1; // tm_mon is zero-indexed
  return (int)difftime(mktime(&timerTime), mktime(&currentTime));
}

static void setWakeUpTimer(int wHours, int wMinutes, struct tm currentTime) {
  struct tm wakePeriod = currentTime;
  wakePeriod.tm_hour += wHours;
  wakePeriod.tm_min += wMinutes;
  int seconds = (int)difftime(mktime(&wakePeriod), mktime(&currentTime));
  esp_sleep_enable_timer_wakeup(1000000LL * seconds);
}

static void waitingScreen(Inkplate &display) {
  display.setTextSize(1);
  display.printf("\n           ");
  display.printf("Waiting for: ");
  display.setTextSize(4);
  display.setCursor(0, 33);
  display.printf("  ");
  display.printf("%2.1d:%02d", alarmHour, alarmMins);
  display.setCursor(0, 65);
  display.setTextSize(2);
  display.printf("    ");
  display.printf("on %2.1d.%2.1d.", alarmDay, alarmMon);
  display.display();
}

static void alarmScreen(Inkplate &display) {
  display.setTextSize(2);
  display.setCursor(9, 30);
  display.printf("ALARM!\n");
  display.setTextSize(1);
  display.display();
}

extern "C" void app_main(void) {
  static Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(INKPLATE2_BLACK);

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  wifi.setCurrentTime();

  struct tm currentTime;
  getTime(&currentTime, TIME_ZONE);

  display.clearDisplay();

  int secsUntilAlarm = secondsUntilAlarm(alarmHour, alarmMins, alarmSecs,
                                         alarmDay, alarmMon, currentTime);

  if (secsUntilAlarm <= 0) {
    alarmScreen(display);

    // Alarm reached — stay awake
    while (true)
      vTaskDelay(pdMS_TO_TICKS(100));
  } else {
    waitingScreen(display);
    setWakeUpTimer(0, wakeMinutes, currentTime);
    esp_deep_sleep_start();
  }
}