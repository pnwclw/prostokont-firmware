/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple HTTP web content fetch example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to connect Inkplate 10 to a WiFi network,
 *              perform a basic HTTP request to retrieve data from the Internet,
 *              and display the received content on the e-paper display.
 *              This example does NOT parse HTML content; it simply prints the
 *              raw HTTP response body on the screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Build and flash to Inkplate 10.
 * 2) The board connects to the WiFi network.
 * 3) Data is fetched from a remote web server using HTTP.
 * 4) The received content is printed on the e-paper display.
 *
 * Expected output:
 * - Inkplate display shows raw text/HTML fetched from the web.
 *
 * Notes:
 * - This example is intended to demonstrate basic HTTP communication only.
 * - No HTML parsing or content extraction is performed.
 * - Displaying large responses may require text size adjustments.
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
#include "WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "HTTP_REQUEST";

extern "C" void app_main(void) {
  static Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
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