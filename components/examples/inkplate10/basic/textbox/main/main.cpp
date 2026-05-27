/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       TextBox usage example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to use the drawTextBox() function to render
 *              multi-line text inside a defined rectangular area.
 *              The example shows:
 *              - A basic TextBox with default parameters.
 *              - A fully customized TextBox using a custom font,
 *                text scaling, spacing, and optional border.
 *
 *              If a word does not fit at the end of a row, it automatically
 *              wraps to the next line. If the text exceeds the lower boundary
 *              of the box, it ends with three dots (...) to indicate that
 *              not all text is displayed.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      Custom font file (e.g. Roboto_Light_36.h)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Build and flash to Inkplate 10.
 *
 * Expected output:
 * - Two text boxes rendered on the screen:
 *   1) Default TextBox
 *   2) Custom styled TextBox using Roboto font
 *
 * Notes:
 * - This example runs in 1-bit (black & white) mode.
 * - Some custom fonts are drawn bottom-to-top and may require
 *   a vertical offset for correct positioning.
 * - Always call display.display() after drawing operations
 *   to update the physical e-paper screen.
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