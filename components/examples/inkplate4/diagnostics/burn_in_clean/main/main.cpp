/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Burn-in cleaning utility for Soldered Inkplate 4TEMPERA.
 *
 * @details     Runs the built-in burn-in cleaning sequence to remove ghosting
 * and image retention from the e-paper panel. The screen is cycled through
 *              CLEAR_CYCLES full black/white refresh cycles with a delay
 * between each cycle. When the cleaning is complete, a confirmation message is
 *              shown on the display.
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
 * 2) The cleaning sequence runs automatically (20 cycles by default).
 * 3) "Clearing done." is shown when complete.
 *
 * Expected output:
 * - The screen flashes black and white 20 times, then shows "Clearing done."
 *
 * Notes:
 * - CYCLES_DELAY should not be set below 5000 ms between cycles.
 * - Increase CLEAR_CYCLES for more severe ghosting.
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

// Nubmer of clear cycles.
#define CLEAR_CYCLES 20

// Delay between clear cycles (in milliseconds)
// NOTE: cycles delay should not be smaller than 5 seconds
#define CYCLES_DELAY 5000

extern "C" void app_main(void) {
  Inkplate inkplate;
  inkplate.clearDisplay(); // Clear any data that may have been in (software)
                           // frame buffer.

  int cycles = CLEAR_CYCLES;

  // Clean the screen by running the burn in function which starts the cleaning
  // sequence
  inkplate.cleanBurnIn(cycles, CYCLES_DELAY);

  // Print text when clearing is done.
  inkplate.setTextSize(4);
  inkplate.setCursor(100, 100);
  inkplate.print("Clearing done.");
  inkplate.display();
}