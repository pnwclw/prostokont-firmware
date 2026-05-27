/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       EEPROM read/write example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to use the built-in EEPROM on Inkplate 10 to
 *              store data that persists across resets and power cycles.
 *              The example shows how to safely clear, write, and read user data
 *              from EEPROM, while respecting reserved address ranges used by
 *              the e-paper waveform data.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Build and flash to Inkplate 10.
 * 2) The display will first clear user EEPROM data.
 * 3) Sample data is written to EEPROM.
 * 4) Stored data is read back and shown on the display.
 *
 * Expected output:
 * - Messages indicating EEPROM clearing, writing, and reading.
 * - A list of values read from EEPROM displayed on the screen.
 *
 * Notes:
 * - EEPROM addresses 0–75 are reserved for waveform data.
 * - DO NOT read from or write to addresses below 76.
 * - Changing EEPROM size may erase waveform data and affect display operation.
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

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.setTextSize(6);
  display.println("Clearing NVS...");
  display.display();
  clearNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Writing data to NVS...");
  display.partialUpdate(false, false);
  writeNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Reading data from NVS:");
  display.partialUpdate(false, false);
  display.setTextSize(3);
  printNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  return;
}