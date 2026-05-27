/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       WiFi HTTP POST request example using webhook.site (Inkplate 10).
 *
 * @details     Demonstrates how to connect Inkplate 10 to a WiFi network and
 *              send periodic HTTP POST requests to webhook.site. This free
 *              online service allows real-time inspection of HTTP requests,
 *              making it useful for testing IoT data transmission.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      Stable WiFi connection, webhook.site URL
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Visit https://webhook.site and copy your unique webhook URL.
 * 2) Paste only the path part (e.g. "/abcd-1234-efgh") into WEBHOOK_PATH.
 * 3) Build and flash to Inkplate 10.
 * 4) Open Serial Monitor to observe connection status.
 * 5) Watch incoming POST requests live on webhook.site.
 *
 * Expected output:
 * - Inkplate display shows example information.
 * - Serial Monitor logs WiFi connection and POST status.
 * - webhook.site displays incoming POST requests every 20 seconds.
 *
 * Notes:
 * - Example uses HTTP (port 80) for simplicity.
 * - Data is sent in URL-encoded format (application/x-www-form-urlencoded).
 * - Replace example data with real sensor readings if needed.
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
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "HTTP_POST";

#define POSTING_INTERVAL_MS (20ULL * 1000ULL)
#define WEBHOOK_HOST "webhook.site"
#define WEBHOOK_PATH "/YOUR-UNIQUE-WEBHOOK-ID"

static void sendPost() {
  // Build POST body with a random value
  char postData[64];
  int value = esp_random() % 40;
  snprintf(postData, sizeof(postData), "value=%d", value);

  char url[128];
  snprintf(url, sizeof(url), "http://%s%s", WEBHOOK_HOST, WEBHOOK_PATH);

  esp_http_client_config_t config = {};
  config.url = url;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = 10000;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    return;
  }

  esp_http_client_set_header(client, "Content-Type",
                             "application/x-www-form-urlencoded");
  esp_http_client_set_header(client, "User-Agent", "Inkplate-ESP32");
  esp_http_client_set_post_field(client, postData, strlen(postData));

  esp_err_t ret = esp_http_client_perform(client);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "POST sent: %s  status=%d", postData,
             esp_http_client_get_status_code(client));
  } else {
    ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(ret));
  }

  esp_http_client_cleanup(client);
}

extern "C" void app_main(void) {
  static Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.setTextWrap(true);

  display.setTextSize(6);
  display.setCursor(0, 0);
  display.print("HTTP POST example\n\nUsing webhook.site");
  display.display();

  // Connect using existing WiFi class (SSID/pass via menuconfig)
  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    display.print("WiFi failed!");
    display.display();
    return;
  }

  ESP_LOGI(TAG, "WiFi connected");

  while (true) {
    sendPost();
    vTaskDelay(pdMS_TO_TICKS(POSTING_INTERVAL_MS));
  }
}