/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       HTTP POST request example for Soldered Inkplate 6Color.
 *
 * @details     Connects to a WiFi network, then periodically sends an HTTP POST
 *              request with a random integer value to a webhook.site endpoint.
 *              The webhook URL must be configured by updating WEBHOOK_PATH.
 *              Posts are sent every 20 seconds.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      WiFi network with internet access; webhook.site account
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 * - Menuconfig -> WiFi Settings -> SSID and Password
 *
 * How to use:
 * 1) Create a free webhook at https://webhook.site and copy your unique URL
 * path. 2) Set WEBHOOK_PATH to your unique path and configure WiFi via
 * menuconfig. 3) Build and flash to Inkplate 6Color. 4) Watch incoming POST
 * requests on webhook.site.
 *
 * Expected output:
 * - "HTTP POST example" shown on the display; POST data logged via serial.
 *
 * Notes:
 * - Change POSTING_INTERVAL_MS to adjust the posting frequency.
 * - Only HTTP (not HTTPS) is used in this example.
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
  Inkplate display;
  display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
  display.setTextWrap(true);

  display.setTextSize(3);
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