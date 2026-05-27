/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read text from a microSD card for Soldered Inkplate 6Color.
 *
 * @details     Initialises the microSD card, reads the first line of a text
 *              file named message.txt from the card root, and displays the
 *              content on the e-paper screen. The result is also printed via
 *              ESP-IDF logging.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable, microSD card
 * - Extra:      A file named message.txt in the root of the microSD card
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Create a file message.txt on a FAT-formatted microSD card.
 * 2) Insert the card, build and flash to Inkplate 6Color.
 * 3) The display shows the contents of message.txt.
 *
 * Expected output:
 * - "From SD card:" followed by the first line of message.txt on the display.
 *
 * Notes:
 * - The SD card must be formatted as FAT32.
 * - Use the microsd/write example first to create the message.txt file.
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

  // read from file
  FILE *f = fopen("/sdcard/message.txt", "r");
  char buf[1024] = {};

  if (!f) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }
  fgets(buf, sizeof(buf), f);
  fclose(f);
  ESP_LOGI(TAG, "Read: %s", buf);

  // Display the text on the e-ink screen
  display.clearDisplay();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.print("From SD card:");
  display.setCursor(10, 40);
  display.print(buf);
  display.display();
}