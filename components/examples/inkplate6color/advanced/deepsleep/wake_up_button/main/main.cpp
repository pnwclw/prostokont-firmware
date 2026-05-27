/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Deep sleep with button or timer wakeup for Soldered Inkplate
 * 6Color.
 *
 * @details     Demonstrates two wakeup sources for ESP32 deep sleep: a
 * 30-second timer and the hardware wake-up button (GPIO 36). On each wakeup the
 * boot count (stored in RTC memory) is incremented and the wakeup cause is
 * printed on the display. After updating the screen the board re-enters deep
 * sleep.
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
 * 2) The board shows the boot count and wakeup reason, then sleeps for 30
 * seconds. 3) Press the wake-up button to wake the board early.
 *
 * Expected output:
 * - Boot count and wakeup reason ("timer" or "WakeUp button") displayed on
 * screen.
 *
 * Notes:
 * - RTC_DATA_ATTR keeps the boot counter in RTC memory across deep sleep
 * cycles.
 * - The wake-up button is connected to GPIO 36.
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
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TIME_TO_SLEEP_US (30ULL * 1000000ULL)

// Store int in rtc data, to remain persistent during deep sleep
RTC_DATA_ATTR static int bootCount = 0;

// Function that will write number of boots and boot reason to screen
static void displayInfo(Inkplate &display) {
  // First, lets delete everything from frame buffer
  display.clearDisplay();

  // Set text cursor and size
  display.setCursor(10, 180);
  display.setTextSize(2);

  // Set next line cursor position
  display.print("Boot count: ");
  display.print(bootCount);

  // Set next line cursor position
  display.setCursor(10, 220);

  // Display wake up reason
  switch (esp_sleep_get_wakeup_cause()) {
  case ESP_SLEEP_WAKEUP_EXT0:
    display.print("Wakeup caused by WakeUp button");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    display.print("Wakeup caused by timer");
    break;
  default:
    display.print("Wakeup was not caused by deep sleep");
    break;
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextColor(INKPLATE_BLACK);

  ++bootCount;

  // Our function declared below
  displayInfo(display);

  // Go to sleep for TIME_TO_SLEEP seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  // Enable wakeup from deep sleep on gpio 36 (wake button)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
  // Go to sleep
  esp_deep_sleep_start();
}