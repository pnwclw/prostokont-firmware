/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Adafruit GFX drawing showcase for Inkplate 2 tri-color palette
 *              (black/white/red), including shapes, text, and rotation.
 *
 * @details     This example demonstrates a wide range of Adafruit
 * GFX-compatible drawing primitives on Inkplate 2 using the BLACK/WHITE/RED
 * color palette. The sketch renders a sequence of demonstrations, each drawn
 * into the framebuffer and then pushed to the panel with a full refresh:
 *              - Text with shadow
 *              - Single pixels and random pixels
 *              - Lines (including thick, horizontal, vertical, and random)
 *              - Grids
 *              - Rectangles (outlined/filled) and rounded rectangles
 *              - Circles (outlined/filled)
 *              - Triangles (outlined/filled)
 *              - Ellipses (outlined/filled)
 *              - Polygons (outlined/filled), including point sorting
 *
 *              A small helper (displayCurrentAction) prints a caption
 * describing the current demo step. At the end, the sketch enters an infinite
 *              loop that rotates the screen (0/90/180/270 degrees) and redraws
 *              text at each orientation.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 2) The display cycles through many drawing examples, pausing for DELAY_MS
 *    between steps.
 * 3) At the end, the sketch continuously rotates the display and redraws text.
 *
 * Expected output:
 * - Display: sequential pages showing different graphics primitives and text,
 *   with a small caption indicating the current action.
 * - Final stage: the text "INKPLATE2" repeatedly redrawn with rotation changes.
 *
 * Notes:
 * - Display mode is 1-bit with Inkplate 2 tri-color palette (BLACK/WHITE/RED).
 * - This example uses full refresh (display()) for every step, so frequent
 *   flashing is expected and updates may take noticeable time.
 * - fillPolygon() can be slower than other primitives; complexity grows with
 *   the number of vertices and the filled area.
 * - Random drawing uses random() without an explicit seed, so patterns may
 *   repeat between resets.
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

#include "esp_random.h" // esp_random()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h> // atan2()

#include "Inkplate.h"

#define DELAY_MS 5000

static int32_t rnd(int32_t min, int32_t max) {
  return min + (int32_t)(esp_random() % (uint32_t)(max - min));
}

// Small function that will write on the screen what function is currently in
// demonstration.
void displayCurrentAction(Inkplate *display, const char *text) {
  display->setTextSize(1);
  display->setCursor(2, 85);
  display->print(text);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay(); // Clear any data that may have been in (software)
                          // frame buffer.
  //(NOTE! This does not clean image on screen, it only clears it in the frame
  // buffer inside ESP32).
  display.display(); // Clear everything that has previously been on a screen
  display.setCursor(10, 10);
  display.setTextSize(2);
  display.drawTextWithShadow(0, 30, "Welcome to", INKPLATE2_RED,
                             INKPLATE2_BLACK); // Draw text with shadow
  display.drawTextWithShadow(0, 50, "Inkplate 2!", INKPLATE2_RED,
                             INKPLATE2_BLACK); // Draw text with shadow
  display.display();                           // Write hello message
  vTaskDelay(pdMS_TO_TICKS(5000));             // Wait a little bit

  // Example will demostrate funcionality one by one. You always first set
  // everything in the frame buffer and afterwards you show it on the screen
  // using display.display().

  // Let's start by drawing a pixel at x = 100 and y = 50 location
  display
      .clearDisplay(); // Clear everytning that is inside frame buffer in ESP32
  displayCurrentAction(
      &display,
      "Drawing two pixels in different colors"); // Function which writes small
                                                 // text at bottom left
                                                 // indicating what's currently
                                                 // done
  // NOTE: you do not need displayCurrentAction function to use Inkplate!
  display.drawPixel(100, 50,
                    INKPLATE2_BLACK); // Draw one black pixel at X = 100, Y = 50
                                      // position in INKPLATE2_BLACK color

  display.drawPixel(50, 100,
                    INKPLATE2_RED); // Draw one black pixel at X = 50, Y = 100
                                    // position in INKPLATE2_BLACK color

  display.display(); // Send image to display. You need to call this one each
                     // time you want to transfer frame buffer
  // to the screen.
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

  // Now, let's draw some random pixels!
  display
      .clearDisplay(); // Clear everything that is inside frame buffer in ESP32
  for (int i = 0; i < 300; i++) { // Write 300 black pixels at random locations
    display.drawPixel(rnd(0, 211), rnd(0, 80), rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing 300 random pixels");
  display.display(); // Write everything from frame buffer to screen
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait

  // Draw two diagonal lines accros screen
  display.clearDisplay();

  // All of those drawing fuctions originate from Adafruit GFX library, so maybe
  // you are already familiar with those. Arguments are: start X, start Y,
  // ending X, ending Y, color.
  display.drawLine(0, 0, 211, 80, INKPLATE2_BLACK);
  display.drawLine(211, 0, 0, 80, INKPLATE2_BLACK);
  displayCurrentAction(&display, "Drawing two diagonal lines");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // And again, let's draw some random lines on screen!
  display.clearDisplay();
  for (int i = 0; i < 30; i++) {
    display.drawLine(rnd(0, 211), rnd(0, 80), rnd(0, 211), rnd(0, 80),
                     rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing 30 random lines");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Let's draw some random thick lines on screen!
  display.clearDisplay();
  for (int i = 0; i < 20; i++) {
    display.drawThickLine(rnd(0, 211), rnd(0, 80), rnd(0, 211), rnd(0, 80),
                          INKPLATE2_BLACK, (float)rnd(1, 20));
  }
  displayCurrentAction(&display, "Drawing 20 thick lines");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Now draw one horizontal...
  display.clearDisplay();
  display.drawFastHLine(
      100, 80, 104,
      INKPLATE2_BLACK); // Arguments are: starting X, starting Y, length, color
  displayCurrentAction(&display, "Drawing one horizontal line");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  //... and one vertical line
  display.clearDisplay();
  display.drawFastVLine(
      100, 10, 70,
      INKPLATE2_RED); // Arguments are: starting X, starting Y, length, color
  displayCurrentAction(&display, "Drawing one vertical line");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Now, let's make a grid using only horizontal and vertical lines
  display.clearDisplay();
  for (int i = 0; i < 212; i += 8) {
    display.drawFastVLine(i, 0, 80, INKPLATE2_BLACK);
  }
  for (int i = 0; i < 81; i += 4) {
    display.drawFastHLine(0, i, 212, INKPLATE2_RED);
  }
  displayCurrentAction(&display,
                       "Drawing a grid using horizontal and vertical lines");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw rectangle at X = 20, Y = 20 and size of 100x35 pixels
  display.clearDisplay();
  display.drawRect(20, 20, 100, 35,
                   INKPLATE2_BLACK); // Arguments are: start X, start Y, size X,
                                     // size Y, color
  displayCurrentAction(&display, "Drawing rectangle");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw rectangles on random location, size 10x15 pixels
  display.clearDisplay();
  for (int i = 0; i < 20; i++) {
    display.drawRect(rnd(0, 211), rnd(0, 40), 10, 15, rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing many rectangles");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw filled black rectangle at X = 20, Y = 20, size of 40x30 pixels
  display.clearDisplay();
  display.fillRect(20, 20, 40, 30,
                   INKPLATE2_BLACK); // Arguments are: start X, start Y, size X,
                                     // size Y, color
  displayCurrentAction(&display, "Drawing black rectangle");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw filled rectangles on random location, size of 30x30 pixels in
  // different colors
  display.clearDisplay();
  for (int i = 0; i < 20; i++) {
    display.fillRect(rnd(0, 211), rnd(0, 40), 30, 20, rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing many filled rectangles randomly");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw circle at center of a screen with radius of 20 pixels
  display.clearDisplay();
  display.drawCircle(
      80, 50, 20,
      INKPLATE2_RED); // Arguments are: start X, start Y, radius, color
  displayCurrentAction(&display, "Drawing a circle");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw some circles at random location with radius of 20 pixels
  display.clearDisplay();
  for (int i = 0; i < 15; i++) {
    display.drawCircle(rnd(0, 211), rnd(0, 40), 20, rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing many circles randomly");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw black filled circle at center of a screen with radius of 20 pixels
  display.clearDisplay();
  display.fillCircle(
      80, 40, 20,
      INKPLATE2_BLACK); // Arguments are: start X, start Y, radius, color
  displayCurrentAction(&display, "Drawing black-filled circle");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw some black filled circles at random location with radius of 10 pixels
  display.clearDisplay();
  for (int i = 0; i < 40; i++) {
    display.fillCircle(rnd(0, 211), rnd(0, 40), 10, rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing many filled circles randomly");
  display.display(); // To show stuff on screen, you always need to call
                     // display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw rounded rectangle at X = 20, Y = 20 and size of 40x30 pixels and
  // radius of 10 pixels
  display.clearDisplay();
  display.drawRoundRect(20, 20, 40, 30, 10,
                        INKPLATE2_RED); // Arguments are: start X, start Y, size
                                        // X, size Y, radius, color
  displayCurrentAction(&display, "Drawing rectangle with rounded edges");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw rounded rectangles on random location, size 30x20 pixels, radius of 5
  // pixels
  display.clearDisplay();
  for (int i = 0; i < 20; i++) {
    display.drawRoundRect(rnd(0, 211), rnd(0, 40), 30, 20, 5, rnd(1, 3));
  }
  displayCurrentAction(&display, "Drawing many rounded edges rectangles");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw filled black rect at X = 20, Y = 20, size of 40x30 pixels and radius
  // of 6 pixels
  display.clearDisplay();
  display.fillRoundRect(20, 20, 40, 30, 6,
                        INKPLATE2_BLACK); // Arguments are: start X, start Y,
                                          // size X, size Y, radius, color
  displayCurrentAction(&display, "This is filled rectangle with rounded edges");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw filled rects on random location, size of 15x15 pixels, radius of 3
  // pixels with random color
  display.clearDisplay();
  for (int i = 0; i < 20; i++) {
    display.fillRoundRect(rnd(0, 211), rnd(0, 60), 15, 15, 3, rnd(1, 3));
  }
  displayCurrentAction(&display, "Random rounded edge filled rectangles");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw simple triangle
  display.clearDisplay();
  display.drawTriangle(
      25, 40, 55, 40, 40, 10,
      INKPLATE2_RED); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draw filled triangle inside simple triangle (so no display.clearDisplay()
  // this time)
  display.fillTriangle(
      30, 35, 50, 35, 40, 15,
      INKPLATE2_BLACK); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
  displayCurrentAction(&display,
                       "Drawing filled triangle inside exsisting one");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Draws an elipse with x radius, y radius, center x, center y and color
  display.clearDisplay();
  display.drawElipse(50, 15, 40, 30, INKPLATE2_RED);
  displayCurrentAction(&display, "Drawing an elipse");
  display.display();

  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Fills an elipse with x radius, y radius, center x, center y and color
  display.clearDisplay();
  display.fillElipse(50, 15, 40, 30, INKPLATE2_RED);
  displayCurrentAction(&display, "Drawing a filled elipse");
  display.display();

  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Code block for generating random points and sorting them in a counter
  // clockwise direction.
  int xt[10];
  int yt[10];
  int n = 10;
  for (int i = 0; i < n; ++i) {
    xt[i] = rnd(10, 200);
    yt[i] = rnd(10, 80);
  }
  int k;
  for (int i = 0; i < n - 1; ++i)
    for (int j = i + 1; j < n; ++j)
      if (atan2(yt[j] - 35, xt[j] - 105) < atan2(yt[i] - 35, xt[i] - 105)) {
        k = xt[i], xt[i] = xt[j], xt[j] = k;
        k = yt[i], yt[i] = yt[j], yt[j] = k;
      }

  // Draws a polygon, from x and y coordinate arrays of n points in color c
  display.clearDisplay();
  display.drawPolygon(xt, yt, n, INKPLATE2_BLACK);
  displayCurrentAction(&display, "Drawing a polygon");
  display.display();

  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Fills a polygon, from x and y coordinate arrays of n points in color c,
  // Points need to be counter clockwise sorted
  // Method can be quite slow for now, probably will improve
  display.clearDisplay();
  display.fillPolygon(xt, yt, n, INKPLATE2_RED);
  displayCurrentAction(&display, "Drawing a filled polygon");
  display.display();

  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  // Write text and rotate it by 90 deg. forever
  int r = 0;
  display.setTextSize(3);
  display.setTextColor(INKPLATE2_WHITE, INKPLATE2_BLACK);
  while (true) {
    display.setRotation(r % 4);
    display.setCursor(10, 10);
    display.clearDisplay();
    display.print("INKPLATE2");
    display.display();
    r++;
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
  }
}