/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       NVS (EEPROM-like) storage example for Soldered Inkplate 6Color.
 *
 * @details     Demonstrates non-volatile storage using the ESP-IDF NVS library,
 *              which emulates Arduino-style EEPROM. The example clears the NVS
 *              namespace, writes 128 byte values (0-127) under individual keys,
 *              then reads them back and displays all values on the screen using
 *              a partial update.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Build and flash to Inkplate 6Color.
 * 2) The display shows write and read progress, then all 128 values.
 *
 * Expected output:
 * - Status messages for clearing, writing, and reading NVS.
 * - All 128 stored values (0-127) printed on the display.
 *
 * Notes:
 * - NVS is stored in a dedicated flash partition and survives reboots.
 * - partialUpdate() is used for the final read output to reduce flicker.
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

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "EEPROM_EXAMPLE";

#define DATA_SIZE 128
#define NVS_NAMESPACE "user_data"

static void clearNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  // Erase every key we own (key names "d000" … "d127")
  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);
    nvs_erase_key(handle, key); // ignore NOT_FOUND errors
  }

  nvs_commit(handle);
  nvs_close(handle);
  ESP_LOGI(TAG, "NVS cleared.");
}

static void writeNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);
    err = nvs_set_u8(handle, key, (uint8_t)i);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "nvs_set_u8[%d] failed: %s", i, esp_err_to_name(err));
    }
  }

  nvs_commit(handle);
  nvs_close(handle);
  ESP_LOGI(TAG, "NVS written.");
}

static void printNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);

    uint8_t val = 0;
    err = nvs_get_u8(handle, key, &val);
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "nvs_get_u8[%d] failed: %s", i, esp_err_to_name(err));
      val = 0;
    }

    display.print(val);
    if (i != DATA_SIZE - 1)
      display.print(", ");
  }

  nvs_close(handle);
  display.partialUpdate(false, false);
}

extern "C" void app_main(void) {
  Inkplate display;

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // Partition was truncated or version mismatch — erase and retry.
    ESP_LOGW(TAG, "NVS partition problem (%s), erasing…", esp_err_to_name(err));
    nvs_flash_erase();
    nvs_flash_init();
  }

  display.setTextColor(INKPLATE_BLACK); // Set text color to black
  display.clearDisplay();

  display.setTextSize(3);
  display.println("Clearing NVS...");
  clearNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Writing data to NVS...");
  writeNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Reading data from NVS:");
  display.setTextSize(2);
  printNVS(display);
  display.display();

  return;
}