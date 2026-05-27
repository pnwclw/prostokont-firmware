/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Partial e-paper update with ESP32 deep sleep for Soldered
 * Inkplate 10.
 *
 * @details     Demonstrates how to correctly use partial screen updates
 * together with ESP32 deep sleep on Inkplate 10. Since partial updates rely on
 * previously stored screen content in RAM, the screen must be recreated after
 * waking from deep sleep before calling partialUpdate(). This example shows how
 * to preserve variables in RTC memory, rebuild the screen, and safely perform
 * partial updates.
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
 * 2) After first full refresh, the device will enter deep sleep.
 * 3) Every 10 seconds the ESP32 wakes up, updates variables,
 *    rebuilds the screen buffer, and performs a partial update.
 * 4) Observe changing values on the display after each wake cycle.
 *
 * Expected output:
 * - First boot performs a full refresh.
 * - Subsequent wake-ups perform partial updates only.
 * - Counter and decimal value increment after each deep sleep cycle.
 *
 * Notes:
 * - Partial update works only in 1-bit (black & white) mode.
 * - Do NOT use standard partial update examples together with deep sleep.
 * - Always rebuild the screen content after deep sleep before calling
 * partialUpdate().
 * - It is recommended to perform a full refresh every 5–10 partial updates
 *   to maintain good image quality.
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

#include "esp_attr.h" // RTC_DATA_ATTR
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/rtc.h" // rtc_get_reset_reason(), DEEPSLEEP_RESET

#include "Inkplate.h"

// 10 seconds in microseconds
#define TIME_TO_SLEEP_US (10ULL * 1000000ULL)

// Counter variable (stored in RTC RAM that stores variable even if deep sleep
// is used) Variables that are changed after each partial update has to be
// stored in RTC RAM in order to recreate screen before deep sleep
RTC_DATA_ATTR static int counter = 0;
RTC_DATA_ATTR static float decimal = 3.14159265f;

static void createScreen(Inkplate *display) {
  display->setFont(NULL);
  display->setTextSize(3); // Set font to be scaled up three times
  display->setTextColor(BLACK, WHITE);

  display->setCursor(200, 300); // Set text cursor @ X = 200, Y = 300
  display->print("First variable:");
  display->print(counter); // Write first variable to buffer

  display->setCursor(200, 340); // Set text cursor @ X = 200, Y = 340
  display->print("Second variable:");
  display->print(
      decimal, 2); // Write second variable to buffer (use two decimals places)
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  createScreen(&display); // Function that contains everything that has to be
                          // written on screen

  if (rtc_get_reset_reason(0) ==
      DEEPSLEEP_RESET) // Check if ESP32 is reseted by deep sleep or power up /
                       // user manual reset (or some other reason)
  {
    display.preloadScreen(); // If is woken up by deep sleep, recreate whole
                             // screen to be same as was before deep sleep
    // Update variable / variables
    counter++;
    decimal *= 1.23f;

    display.clearDisplay(); // Clear everything in buffer
    createScreen(&display); // Create new screen with new variables
    display.partialUpdate(
        true); // Partial update of screen. (Use this only in this
               // scenario, otherwise YOU CAN DAMAGE YOUR SCRREN)
  } else // If is not deep sleep reset, that must be some thing else, so use
         // normal update procedure (full screen update)
  {
    display.display();
  }

  esp_sleep_enable_timer_wakeup(
      TIME_TO_SLEEP_US);  // Set EPS32 to be woken up in 10 seconds (in this
                          // case)
  esp_deep_sleep_start(); // Put ESP32 into deep sleep (low power mode)
}