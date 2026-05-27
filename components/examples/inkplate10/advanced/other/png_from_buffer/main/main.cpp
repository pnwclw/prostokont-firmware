/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Display a PNG image loaded into a RAM buffer on Soldered
 * Inkplate 10.
 *
 * @details     Demonstrates how to read a PNG file from an SD card into a RAM
 *              buffer and then display it using drawPngFromBuffer(). The same
 *              technique applies to PNG data received from any source — a
 * network socket, a serial transfer, a flash partition, etc.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable, microSD card
 * - Extra:      SD card containing a file named "image.png"
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Copy a PNG file named "image.png" to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Build and flash to Inkplate 10.
 * 4) The PNG is read into RAM and rendered on the e-paper display.
 *
 * Expected output:
 * - The PNG image is shown on the Inkplate display.
 *
 * Notes:
 * - The entire PNG file is loaded into heap memory before decoding.
 *   Make sure the file fits in available RAM (ESP32 has ~300 KB free heap).
 * - Dithering is enabled by default; pass false as the fifth argument to
 * disable it.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();

  // Init SD card
  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("SD card error!");
    display.display();
    return;
  }

  // Open the PNG file
  FILE *f = fopen("/sdcard/image.png", "rb");
  if (!f) {
    ESP_LOGE(TAG, "Cannot open image.png");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Cannot open image.png");
    display.display();
    display.sdCardSleep();
    return;
  }

  // Get file size
  fseek(f, 0, SEEK_END);
  long fileSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (fileSize <= 0) {
    ESP_LOGE(TAG, "Invalid file size: %ld", fileSize);
    fclose(f);
    display.sdCardSleep();
    return;
  }

  ESP_LOGI(TAG, "PNG file size: %ld bytes", fileSize);

  // Allocate a buffer large enough for the whole file
  uint8_t *buf = (uint8_t *)malloc((size_t)fileSize);
  if (!buf) {
    ESP_LOGE(TAG, "Not enough RAM for image buffer! Free heap: %lu",
             (unsigned long)esp_get_free_heap_size());
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Not enough RAM!");
    display.display();
    fclose(f);
    display.sdCardSleep();
    return;
  }

  // Read entire file into buffer
  size_t bytesRead = fread(buf, 1, (size_t)fileSize, f);
  fclose(f);
  display.sdCardSleep();

  if (bytesRead != (size_t)fileSize) {
    ESP_LOGE(TAG, "Read mismatch: expected %ld, got %zu", fileSize, bytesRead);
    free(buf);
    return;
  }

  ESP_LOGI(TAG, "File read OK, decoding PNG...");

  // Draw PNG from buffer
  if (!display.image.draw(buf, (uint32_t)fileSize, 0, 0, true, false)) {
    ESP_LOGE(TAG, "PNG decode error");
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("PNG decode error");
  }

  free(buf);
  display.display();

  ESP_LOGI(TAG, "Done");
}