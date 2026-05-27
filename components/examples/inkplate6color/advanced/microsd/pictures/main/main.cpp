/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a BMP image from microSD card on Soldered Inkplate
 * 6Color.
 *
 * @details     Initialises the microSD card and draws a BMP image file stored
 *              in the card root to the e-paper display at position (0, 0).
 *              The image file name is set by the IMAGE_PATH define.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable, microSD card
 * - Extra:      A BMP image file named image1.bmp in the microSD card root
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Convert your image to BMP using
 * https://tools.soldered.com/tools/image-converter/ 2) Copy the BMP file to the
 * root of a FAT-formatted microSD card as image1.bmp. 3) Insert the card, build
 * and flash to Inkplate 6Color.
 *
 * Expected output:
 * - The BMP image displayed at the top-left corner of the e-paper screen.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Change IMAGE_PATH to use a different filename.
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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

// Image file on the SD card root
#define IMAGE_PATH "image1.bmp"

extern "C" void app_main(void) {
  Inkplate display;

  // Init SD card. Display if SD card is init propery or not.
  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE("MAIN", "SD card init failed");
    return;
  }

  display.image.draw(IMAGE_PATH, 0, 0, true, false);
  display.display();
}