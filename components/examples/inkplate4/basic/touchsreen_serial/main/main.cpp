/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Print touch coordinates to serial for Soldered Inkplate
 * 4TEMPERA.
 *
 * @details     Reads touch events from the touchscreen controller and prints
 * the number of fingers detected and their X/Y coordinates to the serial
 *              output. Also draws a reference triangle at the (0,0) corner of
 * the display to help orient coordinates. On release, prints "Release".
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
 * 2) Open a serial monitor at the default baud rate.
 * 3) Touch the screen to print finger count and coordinates.
 *
 * Expected output:
 * - "1 finger X=123 Y=456" (or 2 fingers) printed per touch event.
 * - "Release" printed when all fingers are lifted.
 *
 * Notes:
 * - getData() returns the number of active touch points and fills the x/y
 * arrays.
 * - Up to 2 simultaneous touch points are supported.
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

extern "C" void app_main() {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setCursor(50, 50);
  display.setTextSize(2);
  display.print("Draw on the screen!");
  display.display();

  display.fillTriangle(13, 13, 23, 43, 43, 23, BLACK);
  display.setTextSize(3);
  display.setCursor(65, 65);
  display.print("(0,0) position");
  display.display();

  while (true) {
    if (display.touchscreen.available()) {
      uint8_t n;
      uint16_t x[2], y[2];

      n = display.touchscreen.getData(x, y);
      if (n != 0) {
        printf("%d finger%c ", n, n > 1 ? 's' : ' ');
        for (int i = 0; i < n; i++)
          printf("X=%d Y=%d ", x[i], y[i]);
        printf("\n");
      } else {
        x[0] = x[1] = y[0] = y[1] = 0;
        printf("Release\n");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}