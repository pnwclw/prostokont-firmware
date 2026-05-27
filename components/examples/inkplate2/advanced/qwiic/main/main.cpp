/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       I2C (Qwiic) scanner example for Soldered Inkplate 2.
 *
 * @details     Scans the I2C bus for connected Qwiic/I2C devices and displays
 *              detected device addresses both on the Serial Monitor and on
 *              the Inkplate 2 e-paper display. Useful for validating proper
 *              wiring and confirming device communication.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable, optional Qwiic/I2C device
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 2) Connect a Qwiic/I2C device to the Inkplate.
 * 3) Upload the sketch to Inkplate 2.
 * 4) Open the Serial Monitor (115200 baud).
 * 5) Detected I2C addresses will be shown on both the display and Serial.
 *
 * Expected output:
 * - Inkplate display lists detected I2C device addresses
 * - Serial Monitor logs scanning progress and addresses
 *
 * Notes:
 * - Valid I2C addresses range from 0x01 to 0x7E.
 * - Scan repeats every 5 seconds.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "I2C.h"
#include "Inkplate.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "I2C_SCANNER";

static void scanI2C(Inkplate &display, I2C &i2c) {
  int nDevices = 0;
  int yCursor = 30;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("Scanning I2C...");
  display.setTextSize(1);

  ESP_LOGI(TAG, "Scanning...");

  for (uint8_t address = 1; address < 0x7F; address++) {
    esp_err_t ret = i2c_master_probe(i2c.getBusHandle(), address, 10);

    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "I2C device found at address 0x%02X", address);

      display.setCursor(0, yCursor);
      display.print("Found: 0x");
      if (address < 0x10)
        display.print("0");
      display.print(address, 16);
      yCursor += 12;
      nDevices++;
    }
  }

  if (nDevices == 0) {
    ESP_LOGI(TAG, "No I2C devices found");
    display.setCursor(0, yCursor);
    display.print("No devices found.");
  } else {
    ESP_LOGI(TAG, "Scan done, %d device(s) found", nDevices);
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  // Inkplate2 has no internal I2C use so we init the bus ourselves
  I2C i2c;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(INKPLATE2_BLACK);
  display.setCursor(0, 0);
  display.print("Inkplate I2C Scanner");
  display.display();

  while (true) {
    scanI2C(display, i2c);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}