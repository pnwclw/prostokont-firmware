#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern I2C i2c;

static const char *TAG = "I2C_SCANNER";

static void scanI2C(Inkplate &display) {
  int nDevices = 0;
  int yCursor = 30;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("Scanning I2C...");
  display.setTextSize(1);

  ESP_LOGI(TAG, "Scanning...");

  for (uint8_t address = 1; address < 0x7F; address++) {
    // i2c_master_probe sends a start + address + stop and checks for ACK
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

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 0);
  display.print("Inkplate I2C Scanner");
  display.display();

  while (true) {
    scanI2C(display);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}