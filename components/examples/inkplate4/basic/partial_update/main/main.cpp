/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Scrolling text via partial update for Soldered Inkplate
 * 4TEMPERA.
 *
 * @details     Demonstrates partial display updates by scrolling a text string
 *              across the screen from right to left. Only the changed region is
 *              refreshed each frame, which is significantly faster than a full
 *              display update. A full refresh is forced after every
 *              setFullUpdateThreshold() partial updates to prevent ghosting.
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
 * 2) The text scrolls continuously from right to left.
 *
 * Expected output:
 * - "This is partial update on Inkplate 4 e-paper display!" scrolling across
 * the screen.
 *
 * Notes:
 * - partialUpdate(false, true) keeps e-paper power on for faster successive
 * updates.
 * - A full refresh is triggered automatically after 9 partial updates.
 * - Partial update is only supported in BLACK_AND_WHITE display mode.
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

// Char array where you can store your text that will be scrolled.
const char text[] = "This is partial update on Inkplate 4 e-paper display! :)";

// This variable is used for moving the text (scrolling)
int offset = 600;

// This variable is used to define the number of partial updates before doing a
// full update
int partialUpdates = 9;

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display
  display.setTextColor(
      BLACK,
      WHITE); // Set text color to be black and background color to be white
  display.setTextSize(
      4); // Set text to be 4 times bigger than classic 5x7 px text
  display.setTextWrap(false); // Disable text wraping
  /*
  Set the number of partial updates before doing a full update
  This function forces a full update as the next update to ensure that the cycle
  of partial updates starts from a fully updated screen. The Inkplate class
  keeps a internal counter that increments every time partialUpdate() gets
  called.
  */
  display.setFullUpdateThreshold(partialUpdates);

  while (true) {

    display.clearDisplay();         // Clear content in frame buffer
    display.setCursor(offset, 300); // Set new position for text
    display.print(text);            // Write text at new position

    /*
    //Updates changes parts of the screen without the need to refresh the whole
    display
    //partialUpdate(bool _forced, bool leaveOn)
        _forced		Can force partial update in deep sleep (for advanced
    use) leaveOn 	If set to 1, it will disable turning power supply for
    eink after display update in order to increase refresh time
    */
    display.partialUpdate(false, true);
    offset -= 20; // Move text into new position
    if (offset < 0)
      offset = 800; // Text is scrolled till the end of the screen? Get it back
                    // on the start!
    vTaskDelay(pdMS_TO_TICKS(50)); // Delay between refreshes.
  }
}