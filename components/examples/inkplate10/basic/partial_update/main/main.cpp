/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Partial update text scrolling example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to use partial update functionality on the
 *              Inkplate 10 e-paper display in 1-bit (black & white) mode.
 *              The example scrolls a text string across the screen by updating
 *              only the changed areas using partialUpdate(). A full refresh is
 *              periodically forced (every N partial updates) to maintain good
 *              image quality.
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
 * 2) The display performs an initial full refresh.
 * 3) Text scrolls across the screen using partial updates.
 * 4) A full refresh is automatically forced after a defined number of partial
 * updates.
 *
 * Expected output:
 * - Scrolling text rendered on the display using partial updates.
 * - Periodic full refresh to reduce ghosting and maintain display quality.
 *
 * Notes:
 * - Partial update is available only in 1-bit (black & white) mode.
 * - It is not recommended to use partial update on the first refresh after
 * power-up.
 * - Perform a full refresh every 5–10 partial updates to maintain good picture
 * quality.
 * - partialUpdate(_forced, leaveOn):
 *   - _forced: advanced use (e.g. deep sleep workflows) to force a partial
 * update
 *   - leaveOn: keeps the e-paper power rails enabled between updates for faster
 * refreshes
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
#include "freertos/FreeRTOS.h"

// Char array where you can store your text that will be scrolled.
const char text[] = "This is partial update on Inkplate 10 e-paper display! :)";

// This variable is used for moving the text (scrolling)
int offset = 1200;

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