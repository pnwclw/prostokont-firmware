/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Full-screen color bars example for Soldered Inkplate 6Color.
 *
 * @details     Fills the entire Inkplate 6Color screen with seven vertical
 *              color bars — one for each supported color. This is useful as a
 *              quick visual test to verify that all seven e-paper colors are
 *              rendering correctly.
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
 * 2) The display shows seven vertical color bars.
 *
 * Expected output:
 * - Seven full-height vertical bars in: black, white, green, blue, red, yellow,
 * orange.
 *
 * Notes:
 * - display.display() must be called to update the physical e-paper panel.
 * - Inkplate 6Color supports 7 colors: black, white, green, blue, red, yellow,
 * orange.
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

extern "C" void app_main(void) {
  Inkplate display;

  // Draw a full screen of all colors
  display.fillRect(0, 0, 600 / 7 + 2, 448, INKPLATE_BLACK);
  display.fillRect(1 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_WHITE);
  display.fillRect(2 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_GREEN);
  display.fillRect(3 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_BLUE);
  display.fillRect(4 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_RED);
  display.fillRect(5 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_YELLOW);
  display.fillRect(6 * 600 / 7, 0, 600 / 7 + 2, 448, INKPLATE_ORANGE);

  // Show the Image on the screen
  display.display();
}