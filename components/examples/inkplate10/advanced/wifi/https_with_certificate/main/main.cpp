/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Secure HTTPS image download using a pinned certificate (Inkplate
 * 10).
 *
 * @details     Demonstrates how to securely download and display a BMP image
 *              over HTTPS by providing a trusted certificate for server
 *              validation. The example connects Inkplate 10 to WiFi, applies
 *              a PEM certificate (certificate pinning / trust anchor), then
 *              downloads and renders a BMP image from a website that matches
 *              the provided certificate. It also shows a failed download case
 *              when attempting to load an image from a different host where
 *              the certificate is not valid.
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
 * 1) Provide the correct PEM certificate for the website you want to access.
 * 2) Build and flash to Inkplate 10.
 * 3) The board connects to WiFi, applies the certificate, and downloads a BMP
 * image. 4) A second download attempt demonstrates failure when the certificate
 * does not match.
 *
 * Expected output:
 * - First HTTPS image download succeeds and is displayed.
 * - Second HTTPS image download fails due to invalid certificate for that host,
 *   and an error message is shown on the display.
 *
 * Notes:
 * - This example validates the remote server using the provided certificate.
 * - The certificate must match the target host; it cannot be reused for
 * unrelated domains.
 * - Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth, no
 * compression.
 * - If the certificate is outdated/rotated by the server, you must update it in
 * the sketch.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HTTPS_CERT";

// Certificate for varipass.org (ISRG Root X1)
static const char *certificate =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

extern "C" void app_main(void) {
  static Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setTextWrap(true);
  display.setCursor(0, 0);

  display.print("Connecting to WiFi...");
  display.display();

  WiFi wifi;
  if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000)) {
    display.print("WiFi failed!");
    display.display();
    ESP_LOGE(TAG, "WiFi connection failed");
    return;
  }

  display.print("Connected! Downloading...");
  display.display();

  // Set certificate so downloadFileHTTPS uses it for validation
  wifi.setCertificate(certificate);

  // First download — valid certificate for varipass.org, should succeed
  ESP_LOGI(TAG, "Downloading image with valid certificate...");
  if (!display.image.draw("https://varipass.org/neowise_mono.bmp", 0, 0, false,
                          true)) {
    display.print("Image open error");
    display.display();
    ESP_LOGE(TAG, "First image draw failed");
  } else {
    display.display();
  }

  vTaskDelay(pdMS_TO_TICKS(3000));
  display.clearDisplay();

  // Second download — certificate is pinned to varipass.org so this will fail
  ESP_LOGI(
      TAG,
      "Downloading image with mismatched certificate (expected to fail)...");
  if (!display.image.draw("https://raw.githubusercontent.com/"
                          "SolderedElectronics/Inkplate-Arduino-library"
                          "/master/examples/Inkplate5V2/Advanced/WEB_WiFi/"
                          "Inkplate5V2_Show_JPG_With_HTTPClient/image.jpg",
                          0, 100, true, false)) {
    display.setCursor(0, 100);
    display.print("This image wont load as the certificate is invalid");
    display.display();
    ESP_LOGI(TAG, "Second image failed as expected");
  } else {
    display.display();
  }
}