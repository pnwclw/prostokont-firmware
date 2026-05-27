/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Deep sleep wakeup via RTC alarm for Soldered Inkplate 6Color.
 *
 * @details     Uses the on-board RTC to set an alarm 10 seconds in the future,
 *              then enters ESP32 deep sleep. When the alarm fires, the RTC
 *              pulls GPIO 39 low, waking the board via ext0 wakeup. On each
 *              wake-up the current time is read from the RTC and displayed on
 *              the screen. If the RTC has not been set previously it is
 *              initialised to a hardcoded date and time.
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
 * 1) Build and flash to Inkplate 6Color.
 * 2) The board displays the current time, sets an alarm 10 seconds ahead, then
 * sleeps. 3) After 10 seconds the RTC wakes the board and the cycle repeats.
 *
 * Expected output:
 * - Current date and time shown on screen, updating every 10 seconds.
 *
 * Notes:
 * - The RTC alarm interrupt is connected to GPIO 39.
 * - If the RTC loses power, it re-initialises to the hardcoded time.
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

#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/rtc.h"
#include "time.h"

#include "Inkplate.h"
#include "RTC.h"

static void print2Digits(Inkplate *display, uint8_t d) {
  if (d < 10)
    display->print('0');
  display->print(d, 10);
}

static void printCurrentTime(Inkplate *display) {
  display->setCursor(50, 250);
  display->setTextSize(3);
  display->setTextColor(INKPLATE_BLUE,
                        INKPLATE_WHITE); // Set text color and background

  tm t = {};
  display->rtc.getTime(&t);

  switch (t.tm_wday) {
  case 0:
    display->print("Sunday , ");
    break;
  case 1:
    display->print("Monday , ");
    break;
  case 2:
    display->print("Tuesday , ");
    break;
  case 3:
    display->print("Wednesday , ");
    break;
  case 4:
    display->print("Thursday , ");
    break;
  case 5:
    display->print("Friday , ");
    break;
  case 6:
    display->print("Saturday , ");
    break;
  }

  display->print(t.tm_mday);
  display->print(".");
  display->print(t.tm_mon);
  display->print(".");
  display->print(t.tm_year);
  display->print(". ");

  print2Digits(display, t.tm_hour);
  display->print(':');
  print2Digits(display, t.tm_min);
  display->print(':');
  print2Digits(display, t.tm_sec);
}

extern "C" void app_main(void) {
  Inkplate display;

  // Clear alarm flag from any previous alarm
  display.rtc.clearAlarmFlag();

  // Check if RTC is already is set. If ts not, set time and date
  if (!display.rtc.isSet()) {
    tm t = {};
    t.tm_hour = 13;
    t.tm_min = 30;
    t.tm_sec = 0;
    t.tm_mday = 5;
    t.tm_mon = 12;
    t.tm_year = 2026;
    t.tm_wday = 1;

    display.rtc.setTime(t);

    /* Alternative — set via epoch:
     * display.rtc.setTime((time_t)1589610300); */
  }

  printCurrentTime(&display);
  display.display();

  // Set RTC alarm 10 seconds from now
  time_t now;
  display.rtc.getTime(&now);
  display.rtc.setAlarmEpoch(now + 10);

  // Enable wakup from deep sleep on gpio 39 where RTC interrupt is connected
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  esp_deep_sleep_start();
}