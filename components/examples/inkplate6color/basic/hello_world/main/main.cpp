/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Basic "Hello World" example for Soldered Inkplate 6Color.
 *
 * @details     Demonstrates the most basic usage of the Inkplate 6Color by
 *              initializing the display and printing "Hello World!" on the
 *              e-paper screen. The example uses built-in text rendering
 *              functions fully compatible with the Adafruit GFX library.
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
 * 2) After initialization, "Hello World!" appears on the display.
 *
 * Expected output:
 * - The text "Hello World!" displayed on the Inkplate screen.
 *
 * Notes:
 * - display.clearDisplay() clears only the internal framebuffer.
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

  display.clearDisplay(); // Clear the frame buffer (does NOT clear the physical
                          // screen)
  display.setCursor(10, 10); // Set the text position to (10, 10) pixels
  display.setTextSize(4);    // Set text size to 4 (default is 1)
  display.setTextColor(INKPLATE_BLACK); // Set text color to black
  display.print("Hello World!"); // Print "Hello World!" at the set position
  display.display();             // Refresh the e-paper display to show changes
}