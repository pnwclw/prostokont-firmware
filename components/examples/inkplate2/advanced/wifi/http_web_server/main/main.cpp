/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Standalone WiFi web server example for Soldered Inkplate 2.
 *
 * @details     Demonstrates how to use Inkplate 2 as a simple standalone
 *              WiFi access point and HTTP web server. After connecting a PC,
 *              smartphone, or other WiFi-capable device to the Inkplate’s
 *              access point, a web page can be opened in a browser where text
 *              can be entered and sent directly to the Inkplate e-paper
 * display.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi-capable device with a web browser (PC, laptop, smartphone)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 2) Connect your device to the Inkplate WiFi access point.
 * 3) Open the IP address shown on the Inkplate display in a web browser.
 * 4) Enter text into the web page and press “Send to display”.
 * 5) The submitted text appears on the Inkplate display.
 *
 * Expected output:
 * - Inkplate display shows its IP address and received text.
 * - Web page allows sending custom text to the display.
 *
 * Notes:
 * - This is a basic demonstration of using Inkplate as a web server.
 * - Intended for simple interaction and prototyping.
 * - More advanced web interfaces and logic can be built on top of this example.
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

#include "Inkplate.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "HTTP_SERVER";

#define AP_SSID "Inkplate 2"
#define AP_PASS "Soldered"

static Inkplate *g_display = nullptr;
static char g_userText[512] = {};
static esp_ip4_addr_t g_serverIP = {};

static const char HTML_PAGE[] =
    "<!DOCTYPE html><html><body>"
    "<h2>Inkplate 2 Web Server</h2>"
    "<form action=\"/string\" method=\"GET\">"
    "<input type=\"text\" name=\"text\" style=\"width:300px\">"
    "<input type=\"submit\" value=\"Send to display\">"
    "</form>"
    "</body></html>";

static void printInfo() {
  if (!g_display)
    return;

  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), IPSTR, IP2STR(&g_serverIP));

  g_display->clearDisplay();
  g_display->setTextSize(1);
  g_display->setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);

  g_display->setCursor(5, 16);
  g_display->print("Connect to \"");
  g_display->print(AP_SSID);
  g_display->print("\" WiFi");

  g_display->setCursor(5, 36);
  g_display->print("with pass: ");
  g_display->print(AP_PASS);

  g_display->setCursor(5, 56);
  g_display->print("Open Your web browser and open");

  g_display->setCursor(5, 76);
  g_display->print("http://");
  g_display->print(ipStr);
  g_display->print("/");

  g_display->display();
}

static void printText() {
  if (!g_display)
    return;

  g_display->clearDisplay();
  g_display->setTextSize(1);
  g_display->setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);
  g_display->setCursor(0, 10);
  g_display->print("User text");

  g_display->setTextColor(INKPLATE2_RED, INKPLATE2_WHITE);
  g_display->setCursor(0, 20);
  g_display->print(g_userText);

  g_display->display();
}

static esp_err_t handle_root(httpd_req_t *req) {
  httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t handle_string(httpd_req_t *req) {
  // Extract ?text=... query parameter
  char query[512] = {};
  if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
    char param[512] = {};
    if (httpd_query_key_value(query, "text", param, sizeof(param)) == ESP_OK) {
      // URL-decode the text (spaces sent as + or %20)
      int wi = 0;
      for (int ri = 0; param[ri] && wi < (int)sizeof(g_userText) - 1;
           ri++, wi++) {
        if (param[ri] == '+')
          g_userText[wi] = ' ';
        else if (param[ri] == '%' && param[ri + 1] && param[ri + 2]) {
          char hex[3] = {param[ri + 1], param[ri + 2], 0};
          g_userText[wi] = (char)strtol(hex, nullptr, 16);
          ri += 2;
        } else
          g_userText[wi] = param[ri];
      }
      g_userText[wi] = '\0';
    }
  }

  httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
  printText();
  return ESP_OK;
}

static void startAP() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_t *apNetif = esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  wifi_config_t apConfig = {};
  memcpy(apConfig.ap.ssid, AP_SSID, strlen(AP_SSID));
  memcpy(apConfig.ap.password, AP_PASS, strlen(AP_PASS));
  apConfig.ap.ssid_len = strlen(AP_SSID);
  apConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
  apConfig.ap.max_connection = 4;
  apConfig.ap.channel = 6;

  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_set_config(WIFI_IF_AP, &apConfig);
  esp_wifi_start();

  // Get the AP IP (default is 192.168.4.1)
  esp_netif_ip_info_t ipInfo;
  esp_netif_get_ip_info(apNetif, &ipInfo);
  g_serverIP = ipInfo.ip;

  ESP_LOGI(TAG, "AP started, IP: " IPSTR, IP2STR(&g_serverIP));
}

static httpd_handle_t startHTTPServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;

  httpd_handle_t server = nullptr;
  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return nullptr;
  }

  httpd_uri_t rootUri = {};
  rootUri.uri = "/";
  rootUri.method = HTTP_GET;
  rootUri.handler = handle_root;
  httpd_register_uri_handler(server, &rootUri);

  httpd_uri_t stringUri = {};
  stringUri.uri = "/string";
  stringUri.method = HTTP_GET;
  stringUri.handler = handle_string;
  httpd_register_uri_handler(server, &stringUri);

  ESP_LOGI(TAG, "HTTP server started");
  return server;
}

extern "C" void app_main(void) {
  Inkplate display;
  g_display = &display;
  display.setTextWrap(true);

  startAP();
  startHTTPServer();

  printInfo();

  // Nothing left to do — HTTP server runs in its own task
  while (true)
    vTaskDelay(pdMS_TO_TICKS(1000));
}