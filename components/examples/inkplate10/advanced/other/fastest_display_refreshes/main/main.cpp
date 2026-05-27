/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Faster partial refresh demo by keeping the e-paper panel powered
 * on (Inkplate 10).
 *
 * @details     Demonstrates how to speed up consecutive partial updates by
 *              keeping the e-paper panel power enabled during repeated
 * refreshes. The example scrolls text across the screen using partial updates
 *              while the panel is powered on via einkOn().
 *
 *              Normally, the panel power is automatically enabled before each
 *              refresh and disabled afterward to save energy. Calling einkOn()
 *              keeps the high-voltage e-paper power rails enabled, allowing
 *              multiple partialUpdate() calls to run faster without repeated
 *              power cycling. einkOff() disables the panel power again and
 * should always be called before long idle periods or deep sleep.
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
 * 2) A full refresh is performed once at startup.
 * 3) Text scrolls across the display using fast partial updates.
 * 4) The panel is powered off after the animation completes.
 *
 * Expected output:
 * - Smooth scrolling text using faster partial refreshes.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - Keeping the panel powered on increases power consumption.
 * - Always call einkOff() before entering deep sleep.
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

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display(); // Full refresh once at start

  display.setTextColor(
      BLACK,
      WHITE); // Set text color to be black and background color to be white
  display.setTextWrap(false); // Disable text wraping
  display.setTextSize(4);

  const char *text = "Inkplate partial update scrolling demo";
  int textY = 300;
  int textSpeed = 8;

  while (true) {
    int16_t x1, y1;
    uint16_t w, h;

    // Measure text bounds
    display.getTextBounds(text, 0, textY, &x1, &y1, &w, &h);

    int x = -(int)w; // Start off-screen left

    // Keep panel powered for faster partial updates
    display.einkOn();

    while (x < display.width()) {
      // Clear previous text area
      display.fillRect(0, textY - h, display.width(), h + 10, WHITE);

      // Draw text at new x position
      display.setCursor(x, textY);
      display.print(text);

      // Partial update, panel stays on
      display.partialUpdate(0, 1);

      x += textSpeed;
      vTaskDelay(pdMS_TO_TICKS(80));
    }

    // Power off panel to save energy
    display.einkOff();

    ESP_LOGI(TAG, "Scroll complete, pausing...");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}