/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Frontlight brightness fade example for Soldered Inkplate
 * 4TEMPERA.
 *
 * @details     Demonstrates control of the built-in frontlight by gradually
 *              fading the brightness up from 0 to 63 and then back down to 0,
 *              repeating the fade cycle four times. After the cycles complete,
 *              the frontlight is set to a fixed brightness value.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) The frontlight fades up and down four times, then holds at brightness 31.
 *
 * Expected output:
 * - Frontlight visibly fades in and out four times.
 *
 * Notes:
 * - Brightness range is 0 (off) to 63 (maximum).
 * - frontlight.setState(true) must be called before setBrightness().
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int b = 31;

extern "C" void app_main() {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.frontlight.setState(true);
  display.frontlight.setBrightness(b);

  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < 64; ++i) {
      display.frontlight.setBrightness(i);
      vTaskDelay(pdMS_TO_TICKS(30));
    }
    for (int i = 63; i >= 0; --i) {
      display.frontlight.setBrightness(i);
      vTaskDelay(pdMS_TO_TICKS(30));
    }
  }

  display.frontlight.setBrightness(b);
}