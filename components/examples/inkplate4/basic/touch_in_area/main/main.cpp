/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touch-in-area detection example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates the touchInArea() function by drawing a filled
 * black rectangle on screen and moving it diagonally each time it is touched.
 *              A partial update is used for fast on-screen repositioning. When
 * the rectangle goes off the bottom of the screen it resets to the top-left.
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
 * 2) Touch the black rectangle on screen to move it diagonally.
 *
 * Expected output:
 * - A black rectangle that moves by 100 px diagonally each time it is touched.
 *
 * Notes:
 * - touchInArea(x, y, w, h) returns true when a finger is detected inside the
 * region.
 * - A full refresh is triggered when the rectangle wraps back to the starting
 * position.
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

int x_position = 50;
int y_position = 50;

extern "C" void app_main() {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();

  display.setCursor(90, 280);
  display.setTextSize(3);
  display.print("Touch button example.");
  display.setCursor(60, 350);
  display.print("Touch the black button.");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(3000));

  display.clearDisplay();

  display.fillRect(x_position, y_position, 120, 70, BLACK);
  display.display();

  while (true) {
    if (display.touchscreen.touchInArea(x_position, y_position, 120, 70)) {
      x_position += 100;
      y_position += 100;

      if (y_position < 540) {
        display.clearDisplay();
        display.fillRect(x_position, y_position, 120, 70, BLACK);
        display.partialUpdate();
        vTaskDelay(pdMS_TO_TICKS(100));
      } else {
        x_position = 50;
        y_position = 50;

        display.clearDisplay();
        display.fillRect(x_position, y_position, 120, 70, BLACK);
        display.display();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}