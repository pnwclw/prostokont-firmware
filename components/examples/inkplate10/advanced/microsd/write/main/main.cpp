/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Write text file to SD card on Soldered Inkplate 10.
 *
 * @details     Demonstrates how to initialize the SD card, create a .txt file,
 *              write data into it, and properly close the file using the
 *              Inkplate SD card interface. The example writes a short text
 *              string into "test.txt" stored on a FAT-formatted SD card.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable, microSD card
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Insert a FAT-formatted SD card into the Inkplate.
 * 2) Build and flash to Inkplate 10.
 * 3) The program initializes the SD card.
 * 4) A file named "test.txt" is created (or appended if it exists).
 * 5) The text string defined in the code is written into the file.
 *
 * Expected output:
 * - Status messages shown on the Inkplate display.
 * - File "test.txt" created on the SD card with written content.
 *
 * Notes:
 * - SD card must be properly formatted (FAT/FAT32).
 * - Always close files after writing to prevent corruption.
 * - SD card is put into sleep mode after operation to reduce power consumption.
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

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    return;
  }

  // write to file
  FILE *f = fopen("/sdcard/message.txt", "w");
  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return;
  }
  fprintf(f, "Hello from Inkplate!\n");
  fclose(f);
  ESP_LOGI(TAG, "Write successful");

  display.sdCardSleep();
}