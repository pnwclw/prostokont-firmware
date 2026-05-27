/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive black & white graphics demo for Soldered Inkplate
 * 4TEMPERA.
 *
 * @details     Cycles through a wide variety of Adafruit GFX drawing primitives
 *              in 1-bit (black & white) mode: pixels, lines, thick lines,
 * grids, rectangles, circles, rounded rectangles, triangles, ellipses,
 *              polygons, bitmaps, and text at multiple sizes. Each shape is
 * shown for 5 seconds before moving to the next. The demo ends with text that
 * rotates indefinitely.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      logo.h header with logo bitmap included in the project
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) The display cycles through all drawing primitives automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in black
 * & white.
 *
 * Notes:
 * - display.clearDisplay() clears only the internal framebuffer.
 * - display.display() must be called to update the physical e-paper panel.
 * - DELAY_MS controls how long each shape is shown (default 5000 ms).
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
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "logo.h"
#include "math.h"
#define DELAY_MS 5000

static const char *TAG = "MAIN";

int random(int min, int max) { return (esp_random() % (max - min + 1)) + min; }

void displayCurrentAction(Inkplate &display, const char *text) {
  display.setTextSize(2);
  display.setCursor(20, 560);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.clearDisplay();
  display.display();

  display.setCursor(60, 300);
  display.setTextSize(3);
  display.print("Welcome to Inkplate 4TEMPERA!");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

  while (true) {
    // Single pixel
    display.clearDisplay();
    displayCurrentAction(display, "Drawing a pixel");
    display.drawPixel(100, 50, BLACK);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random pixels
    display.clearDisplay();
    for (int i = 0; i < 600; i++)
      display.drawPixel(random(0, 599), random(0, 599), BLACK);
    displayCurrentAction(display, "Drawing random pixels");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 599, 599, BLACK);
    display.drawLine(599, 0, 0, 599, BLACK);
    displayCurrentAction(display, "Diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawLine(random(0, 599), random(0, 599), random(0, 599),
                       random(0, 599), BLACK);
    displayCurrentAction(display, "50 random lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Thick lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawThickLine(random(0, 599), random(0, 599), random(0, 599),
                            random(0, 599), BLACK, (float)random(1, 20));
    displayCurrentAction(display, "50 random thick lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(100, 300, 400, BLACK);
    displayCurrentAction(display, "Horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(300, 100, 400, BLACK);
    displayCurrentAction(display, "Vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Grid
    display.clearDisplay();
    for (int i = 0; i < 600; i += 10)
      display.drawFastVLine(i, 0, 600, BLACK);
    for (int i = 0; i < 600; i += 10)
      display.drawFastHLine(0, i, 600, BLACK);
    displayCurrentAction(display, "Grid");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rectangle
    display.clearDisplay();
    display.drawRect(150, 200, 300, 200, BLACK);
    displayCurrentAction(display, "Rectangle outline");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled rectangle
    display.clearDisplay();
    display.fillRect(150, 200, 300, 200, BLACK);
    displayCurrentAction(display, "Filled rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Circles
    display.clearDisplay();
    display.drawCircle(300, 300, 90, BLACK);
    display.fillCircle(300, 300, 60, BLACK);
    displayCurrentAction(display, "Circle + filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(150, 200, 300, 200, 12, BLACK);
    display.fillRoundRect(170, 220, 260, 160, 18, BLACK);
    displayCurrentAction(display, "Rounded rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Triangle
    display.clearDisplay();
    display.drawTriangle(150, 400, 450, 400, 300, 100, BLACK);
    display.fillTriangle(200, 350, 400, 350, 300, 150, BLACK);
    displayCurrentAction(display, "Triangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Bitmap logo
    display.clearDisplay();
    display.image.draw(logo, 143, 247, logo_w, logo_h, BLACK);
    displayCurrentAction(display, "Logo");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Text demo
    display.clearDisplay();
    for (int i = 0; i < 5; i++) {
      display.setTextSize(i + 1);
      display.setCursor(90, (i * i * 20));
      display.print("Inkplate 4TEMPERA");
    }
    displayCurrentAction(display, "Text sizes");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Ellipse
    display.clearDisplay();
    display.drawElipse(400, 200, 300, 300, BLACK);
    display.fillElipse(400, 200, 150, 150, BLACK);
    displayCurrentAction(display, "Ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rotate text forever
    int r = 0;
    display.setTextSize(4);
    display.setTextColor(BLACK, WHITE);

    while (true) {
      display.clearDisplay();
      display.setCursor(120, 250);
      display.setRotation(r);
      display.print("Inkplate 4TEMPERA");
      display.display();
      r++;
      vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
  }
}