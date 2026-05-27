/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Text box with word wrap example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates the drawTextBox() function which renders long text
 *              within a defined rectangular region with automatic word
 * wrapping. Two text boxes are shown side by side: one using the default
 *              built-in font, and one using a custom Roboto Light 36 pt font
 *              with configurable line spacing.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Roboto_Light_36.h custom font header included in the project
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Two text boxes with the same sample text appear side by side.
 *
 * Expected output:
 * - Left box: sample text in the default font with word wrap.
 * - Right box: same text in Roboto Light 36 pt with 27 px line spacing.
 *
 * Notes:
 * - Text that exceeds the box bounds is truncated with "...".
 * - Some custom fonts are drawn bottom-to-top and require a vertical offset.
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
#include "Roboto_Light_36.h"

// Define the text you will show in the text box
const char *text =
    "This is an example of a text written in a textbox. When a word doesn't "
    "fit into the current row, it goes to the next one."
    " If the text reaches the lower bound, it ends with three dots (...) to "
    "mark that the text isnt displayed fully";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display

  // Create a text box without any optional parameters
  // x0- x coordinate of upper left corner
  // y0- y coordinate of upper left corner
  // x1- x coordinate of bottom right corner
  // y1- y coordinate of bottom right corner
  // text - text we want to display
  display.drawTextBox(100, 100, 300, 300, text);

  // Create a text box with all parameters
  // x0- x coordinate of upper left corner
  // y0- y coordinate of upper left corner
  // x1- x coordinate of bottom right corner
  // y1- y coordinate of bottom right corner
  // text - text we want to display
  // textSizeMultiplier - by what factor we want to enlarge the size of a font
  // font - address of selected custom font
  // verticalSpacing - how many pixels between each row of text
  // showBorder - Create a visible rectangle around the box
  // fontSize - size of the used font in pt
  int offset =
      32; // Note - some custom fonts are drawn from bottom-to-top which
          // requires an offset, use an offset that best suits the font you use
  display.drawTextBox(400, 100 + offset, 600, 300, text, 1, &Roboto_Light_36,
                      27, false, 36);

  // Display both text boxes
  display.display();
}