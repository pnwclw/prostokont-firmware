/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Draw on screen with touch for Soldered Inkplate 4TEMPERA.
 *
 * @details     Uses the touchscreen to draw on the e-paper display. In
 * DRAW_LINE mode (default) a continuous line is drawn between successive touch
 *              positions. In DRAW_CIRCLE mode (enabled by changing the #define)
 *              a filled circle is drawn at each touch point. Partial updates
 * keep the drawing responsive.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Drag a finger across the screen to draw lines.
 * 3) To switch to circle mode, change #define DRAW_LINE to #define DRAW_CIRCLE.
 *
 * Expected output:
 * - Lines drawn on the e-paper wherever the screen is touched and dragged.
 *
 * Notes:
 * - partialUpdate(false, true) keeps e-paper power on for faster successive
 * updates.
 * - The drawing is not cleared automatically; perform a full refresh to reset.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DRAW_LINE
// #define DRAW_CIRCLE

#ifdef DRAW_LINE
uint16_t xOld, yOld;
#endif

extern "C" void app_main() {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setCursor(50, 50);
  display.setTextSize(2);
  display.print("Draw on the screen!");
  display.display();

  while (true) {
    if (display.touchscreen.available()) {
      uint8_t n;
      uint16_t x[2], y[2];
      n = display.touchscreen.getData(x, y);
      if (n != 0) {
#ifdef DRAW_LINE
        display.drawLine(xOld, yOld, x[0], y[0], BLACK);
        xOld = x[0];
        yOld = y[0];
#endif

#ifdef DRAW_CIRCLE
        display.fillCircle(x[0], y[0], 20, BLACK);
#endif
        display.partialUpdate(false, true);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}