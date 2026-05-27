
/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a pre-converted image on Soldered Inkplate 4TEMPERA.
 *
 * @details     Shows how to display an image that was converted to a C header
 *              file using the Soldered online image converter tool. The image
 *              data is stored in image_ex.h and drawn directly to the display
 *              from the ESP32 flash.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      image_ex.h header with a converted image array included in the
 * project
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Convert your image using https://tools.soldered.com/tools/image-converter/
 * 2) Save the generated header as image_ex.h in the main/ folder.
 * 3) Build and flash to Inkplate 4TEMPERA.
 *
 * Expected output:
 * - The converted image displayed at position (0, 0) on the e-paper screen.
 *
 * Notes:
 * - display.clearDisplay() clears only the internal framebuffer.
 * - display.display() must be called to update the physical e-paper panel.
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

#include "image_ex.h"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay(); // Clear frame buffer of display
  // Display selected picture at location X=0, Y=0.
  display.image.draw(image, 0, 0, image_w, image_h, BLACK);
  display.display(); // Refresh the screen with new picture
}