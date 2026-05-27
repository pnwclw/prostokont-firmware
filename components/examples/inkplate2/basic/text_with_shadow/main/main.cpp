/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Adafruit GFX drawing demo: render tri-color text and a shadowed
 *              string, then enter deep sleep.
 *
 * @details     This example demonstrates basic graphics and text rendering on
 *              Inkplate 2 using Adafruit GFX-compatible drawing functions. It
 *              prints standard text in black and red, then uses
 *              drawTextWithShadow() to render a red string with a black shadow.
 *              An additional small signature line is drawn with inverted
 *              shadow/text colors, and a simple red line is drawn to show other
 *              primitives supported by the library.
 *
 *              Inkplate 2 uses a 1-bit display mode with a tri-color palette
 *              (black/white/red). The sketch performs a single full refresh to
 *              show the drawing and then enters deep sleep. Because deep sleep
 *              resets the ESP32, the program restarts from setup() after any
 *              wake/reset (this sketch does not configure a wake source).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 2) After upload/reset, the display refreshes once and shows the rendered
 *    text, shadow effects, and a line.
 * 3) The board enters deep sleep after updating the display.
 *
 * Expected output:
 * - Display: "Inkplate 2" in black, then in red, then a red "Inkplate 2" with
 *   a black shadow; a small "By soldered.com" line with a shadow; and a red
 *   diagonal line.
 *
 * Notes:
 * - Display mode is 1-bit with Inkplate 2 color palette (BLACK/WHITE/RED).
 * - This example uses a full refresh (display()) for the final output.
 * - Deep sleep restarts the ESP32; no wakeup source is configured here, so a
 *   reset or power cycle is required to run again.
 * - The Inkplate Arduino library is compatible with Adafruit GFX primitives,
 *   making it easy to reuse many existing GFX-based drawing examples.
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
#include "esp_sleep.h"

// Define the text you will show in the text box
const char *text =
    "This is an example of a text written in a textbox. When a word doesn't "
    "fit into the current row, it goes to the next one."
    " If the text reaches the lower bound, it ends with three dots (...) to "
    "mark that the text isnt displayed fully";

extern "C" void app_main(void) {
  Inkplate display;
  display.setTextSize(2);                // Set text size
  display.setTextColor(INKPLATE2_BLACK); // Set text color
  display.setCursor(0, 0);               // Set cursor position

  // Draw black text
  display.print("Inkplate 2"); // Print text

  // Draw red text
  display.setTextColor(INKPLATE2_RED);
  display.setCursor(0, 20);
  display.print("Inkplate 2");

  // Draw text with shadow, specify color for text and background
  // (x coordinate, y coordinate, string to write, text color, shadow color)
  display.drawTextWithShadow(0, 40, "Inkplate 2", INKPLATE2_RED,
                             INKPLATE2_BLACK);

  display.setTextSize(1);
  display.drawTextWithShadow(110, 80, "By soldered.com", INKPLATE2_BLACK,
                             INKPLATE2_RED);

  // Other basic drawing functions supported
  display.drawLine(10, 80, 100, 100, INKPLATE2_RED);

  // Display to screen
  display.display();

  // Go to deep sleep
  esp_deep_sleep_start();
}