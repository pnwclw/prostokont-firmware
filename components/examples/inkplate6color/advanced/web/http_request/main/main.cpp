/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       HTTP GET request example for Soldered Inkplate 6Color.
 *
 * @details     Connects to a WiFi network (credentials set via menuconfig),
 *              performs an HTTP GET request to http://example.com/index.html,
 *              and displays the raw HTML response on the e-paper screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      WiFi network with internet access
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 * - Menuconfig -> WiFi Settings -> SSID and Password
 *
 * How to use:
 * 1) Set your WiFi SSID and password via menuconfig.
 * 2) Build and flash to Inkplate 6Color.
 * 3) The board connects to WiFi, downloads the page, and displays it.
 *
 * Expected output:
 * - Raw HTML of http://example.com shown on the display.
 *
 * Notes:
 * - The response buffer is 32 KB; adjust len in the code for larger pages.
 * - Only HTTP (not HTTPS) is supported in this example.
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
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "HTTP_REQUEST";

extern "C" void app_main(void) {
  static Inkplate display;
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(BLACK, WHITE);
  display.setTextWrap(true);

  // Show initial status
  display.print("Connecting to WiFi...");
  display.display();

  // Connect using existing WiFi class (SSID/pass set via menuconfig)
  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    display.print("WiFi connection failed!");
    display.display();
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  display.print("Connected!");
  display.display();

  // Fetch the page
  int32_t len = 32768; // max buffer size — adjust if needed
  uint8_t *data = wifi.downloadFile("http://example.com/index.html", &len);

  if (!data || len <= 0) {
    display.print("HTTP request failed!");
    display.display();
    ESP_LOGE(TAG, "HTTP request failed");
    return;
  }

  // Null-terminate so we can print as string
  data[len] = '\0';
  ESP_LOGI(TAG, "Received %ld bytes", len);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print((char *)data);
  display.display();

  free(data);
}