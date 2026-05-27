/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Write text to a microSD card for Soldered Inkplate 6Color.
 *
 * @details     Initialises the microSD card and writes the string
 *              "Hello from Inkplate!" to a file named message.txt in the card
 *              root. After writing, the SD card is put to sleep to save power.
 *              The result is logged via ESP-IDF logging.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable, microSD card
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Insert a FAT-formatted microSD card.
 * 2) Build and flash to Inkplate 6Color.
 * 3) The file message.txt is created (or overwritten) on the card.
 *
 * Expected output:
 * - "Write successful" in the serial log; message.txt created on the SD card.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Use the microsd/read example to verify the written file.
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