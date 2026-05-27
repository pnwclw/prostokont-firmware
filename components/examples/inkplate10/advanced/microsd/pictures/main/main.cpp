/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display images from SD card on Soldered Inkplate 10.
 *
 * @details     Demonstrates how to load image files from an SD card and display
 *              them on the Inkplate 10 e-paper display. The example shows how
 * to read supported image formats from a FAT-formatted SD card and render them
 * using the Inkplate graphics library.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      SD card with compatible image files
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Copy supported image files to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 10.
 * 4) The image is read from the SD card and displayed on the e-paper screen.
 *
 * Expected output:
 * - Selected image from the SD card is shown on the Inkplate display.
 *
 * Notes:
 * - Supported formats include BMP, JPEG, and PNG (with library limitations).
 * - Supported color depths: 1-bit (BW), 4-bit, 8-bit, and 24-bit.
 * - Maximum supported resolution is 1200 × 825 pixels.
 * - Images larger than the display resolution will not fit on screen.
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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#include "Inkplate.h"

// Image file on the SD card root
#define IMAGE_PATH "coast.jpg"

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);

  // Init SD card. Display if SD card is init propery or not.
  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    return;
  }

  display.image.draw(IMAGE_PATH, 0, 0, true, false);
  display.display();
}