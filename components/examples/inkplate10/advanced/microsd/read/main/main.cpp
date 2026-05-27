/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read and display text file from SD card on Soldered Inkplate 10.
 *
 * @details     Demonstrates how to open a .txt file from a FAT-formatted SD
 * card and display its contents on the Inkplate 10 e-paper display. The example
 * reads a file named "text.txt" from the SD card and prints its content on
 * screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable, microSD card
 * - Extra:      text.txt file on SD card (≤ 200 characters)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 * - SD card format: FAT / FAT32
 *
 * How to use:
 * 1) Copy a text file named "text.txt" (max 200 characters) to a FAT-formatted
 * SD card. 2) Insert the SD card into the Inkplate. 3) Build and flash to
 * Inkplate 10. 4) The file contents will be read and displayed on the e-paper
 * screen.
 *
 * Expected output:
 * - The contents of text.txt are displayed on the Inkplate display.
 *
 * Notes:
 * - File name must be exactly "text.txt" for this example.
 * - Text length should not exceed 200 characters.
 * - Ensure the required SDFat library is properly installed.
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

  // read from file
  FILE *f = fopen("/sdcard/text.txt", "r");
  char buf[1024] = {};

  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }
  fgets(buf, sizeof(buf), f);
  fclose(f);
  ESP_LOGI(TAG, "Read: %s", buf);

  display.sdCardSleep();

  // Display the text on the e-ink screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.print("From SD card:");
  display.setCursor(10, 40);
  display.print(buf);
  display.display();
}