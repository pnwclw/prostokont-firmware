/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive grayscale graphics demo for Soldered Inkplate
 * 4TEMPERA.
 *
 * @details     Cycles through a wide variety of Adafruit GFX drawing primitives
 *              in 3-bit grayscale mode (8 shades, 0 = black, 7 = white).
 * Includes pixels, lines, thick lines, gradient lines, grids, rectangles,
 *              circles, rounded rectangles, triangles, ellipses, polygons,
 * bitmap images, and text in different sizes and shades. Each shape is shown
 *              for 5 seconds. The demo ends with rotating text.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      image_ex.h header with a grayscale image array included in the
 * project
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) The display cycles through all drawing primitives in grayscale
 * automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in
 * 8-shade grayscale.
 *
 * Notes:
 * - Grayscale mode uses values 0 (black) through 7 (white).
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
#include "image_ex.h" //Include image file that holds gray image data.

#include "esp_random.h"
#include "math.h"

// Helper to keep it clean
int random(int min, int max) { return (esp_random() % (max - min + 1)) + min; }

#define DELAY_MS 5000

void displayCurrentAction(Inkplate &display, const char *text) {
  display.setTextSize(2);
  display.setCursor(20, 560);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.clearDisplay(); // Clear any data that may have been in (software)
                          // frame buffer.
                          //(NOTE! This does not clean image on screen, it only
                          // clears it in the frame buffer inside
                          // ESP32).
  display.setDisplayMode(GRAYSCALE);
  display.display(); // Clear everything that has previously been on a screen
  display.setTextColor(0, 7);
  display.setCursor(70, 300);
  display.setTextSize(3);
  display.print("Welcome to Inkplate 4TEMPERA!");
  display.display();                   // Write hello message
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

  while (true) {
    // Example will demostrate funcionality one by one. You always first set
    // everything in the frame buffer and afterwards you show it on the screen
    // using display.display().

    // Let'sstart by drawing pixel at x = 100 and y = 50 location
    display.clearDisplay(); // Clear everytning that is inside frame buffer in
                            // ESP32
    display.drawPixel(100, 50, 0); // Draw one black pixel at X = 100, Y = 50
                                   // position in 0 (BLACK) color
    displayCurrentAction(
        display,
        "Drawing a pixel"); // Function which writes small text at bottom left
                            // indicating what's currently done NOTE: you do not
                            // need displayCurrentAction function to use
                            // Inkplate!
    display.display(); // Send image to display. You need to call this one each
                       // time you want to transfer frame buffer to the screen.
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

    // Now, let's draw some random pixels!
    display.clearDisplay(); // Clear everything that is inside frame buffer in
                            // ESP32
    for (int i = 0; i < 600;
         i++) { // Write 1000 random colored pixels at random locations
      display.drawPixel(random(0, 599), random(0, 599),
                        random(0, 7)); // We are setting color of the pixels
                                       // using numbers from 0 to 7,
    } // where 0 mens black, 7 white and gray is in between
    displayCurrentAction(display, "Drawing 600 random pixels in random colors");
    display.display(); // Write everything from frame buffer to screen
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait

    // Draw two diagonal lines accros screen
    display.clearDisplay();
    display.drawLine(
        0, 0, 599, 599,
        0); // All of those drawing fuctions originate from Adafruit GFX
            // library, so maybe you are already familiar
    display.drawLine(599, 0, 0, 599, 0); // with those. Arguments are: start X,
                                         // start Y, ending X, ending Y, color.
    displayCurrentAction(display, "Drawing two diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // And again, let's draw some random lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawLine(random(0, 599), random(0, 599), random(0, 599),
                       random(0, 599), random(0, 7));
    }
    displayCurrentAction(display, "Drawing 50 random lines in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Let's draw some random thick lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawThickLine(random(0, 599), random(0, 599), random(0, 599),
                            random(0, 599), random(0, 7), (float)random(1, 20));
    }
    displayCurrentAction(display, "Drawing 50 thick lines in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Let's draw some random gradient thick lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      int startColor = random(0, 7);
      int endColor = random(startColor, 7);
      display.drawGradientLine(random(0, 599), random(0, 599), random(0, 599),
                               random(0, 599), startColor, endColor,
                               (float)random(1, 20));
    }
    displayCurrentAction(display,
                         "Drawing 50 lines, random colors and thickness");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Now draw one horizontal...
    display.clearDisplay();
    display.drawFastHLine(
        100, 300, 400,
        0); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction(display, "Drawing one horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    //... and one vertical line
    display.clearDisplay();
    display.drawFastVLine(
        300, 100, 400,
        0); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction(display, "Drawing one vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Now, let' make a grid using only horizontal and vertical lines in random
    // colors!
    display.clearDisplay();
    for (int i = 0; i < 800; i += 8) {
      display.drawFastVLine(i, 0, 600, (i / 8) & 0x0F);
    }
    for (int i = 0; i < 600; i += 4) {
      display.drawFastHLine(0, i, 800, (i / 8) & 0x0F);
    }
    displayCurrentAction(display, "Drawing a grid with different colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rectangle at X = 150, Y = 200 and size of 300x200 pixels
    display.clearDisplay();
    display.drawRect(
        150, 200, 300, 200,
        0); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction(display, "Drawing rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rectangles on random location, size 100x150 pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawRect(random(0, 599), random(0, 599), 100, 150, random(0, 7));
    }
    displayCurrentAction(display, "Drawing many rectangles in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled black rectangle at X = 150, Y = 200, size of 300x200 pixels
    // in gray color
    display.clearDisplay();
    display.fillRect(
        150, 200, 300, 200,
        4); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction(display, "Drawing gray rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled random colored rectangles on random location, size of 30x30
    // pixels in radnom color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.fillRect(random(0, 599), random(0, 599), 30, 30, random(0, 7));
    }
    displayCurrentAction(display, "Drawing filled rectangles in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw circle at center of a screen with radius of 75 pixels
    display.clearDisplay();
    display.drawCircle(300, 300, 75,
                       0); // Arguments are: start X, start Y, radius, color
    displayCurrentAction(display, "Drawing a circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw some random colored circles at random location with radius of 25
    // pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 40; i++) {
      display.drawCircle(random(0, 599), random(0, 599), 25, random(0, 7));
    }
    displayCurrentAction(display, "Drawing random circles in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw black filled circle at center of a screen with radius of 75 pixels
    display.clearDisplay();
    display.fillCircle(300, 300, 75,
                       0); // Arguments are: start X, start Y, radius, color
    displayCurrentAction(display, "Drawing black-filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw some random colored filled circles at random location with radius of
    // 15 pixels
    display.clearDisplay();
    for (int i = 0; i < 40; i++) {
      display.fillCircle(random(0, 599), random(0, 599), 15, random(0, 7));
    }
    displayCurrentAction(display, "Drawing filled circles in random colors");
    display.display(); // To show stuff on screen, you always need to call
                       // display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rounded rectangle at X = 150, Y = 200 and size of 300x200 pixels and
    // radius of 10 pixels
    display.clearDisplay();
    display.drawRoundRect(
        150, 200, 300, 200, 10,
        0); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction(display, "Drawing rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rounded rectangles on random location, size 100x150 pixels, radius
    // of 5 pixels in radnom color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawRoundRect(random(0, 599), random(0, 599), 100, 150, 5,
                            random(0, 7));
    }
    displayCurrentAction(display, "Drawing many rounded edges rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled random colored rectangle at X = 150, Y = 200 and size of
    // 300x200 pixels and radius of 20 pixels
    display.clearDisplay();
    display.fillRoundRect(
        150, 200, 300, 200, 20,
        0); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction(display,
                         "Drawing filled rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled random colored rectangle on random location, size of 30x30
    // pixels, radius of 3 pixels in radnom color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.fillRoundRect(random(0, 599), random(0, 599), 30, 30, 3,
                            random(0, 7));
    }
    displayCurrentAction(display,
                         "Drawing rounded rectangles with random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw simple triangle
    display.clearDisplay();
    displayCurrentAction(display, "Drawing triangle inside");
    display.drawTriangle(150, 400, 450, 400, 300, 100,
                         0); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled triangle inside simple triangle (so no display.clearDisplay()
    // this time)
    display.fillTriangle(200, 350, 400, 350, 300, 150,
                         0); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    displayCurrentAction(display, "Drawing filled triangle inside");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Display some grayscale image on screen. We are going to display a sample
    // photo in the centre of the display
    display.clearDisplay();
    display.image.draw(picture1, 50, 139, 500, 332,
                       BLACK); // Arguments are: array variable name, start X,
                               // start Y,  size X, size Y
    displayCurrentAction(display, "Drawing a bitmap image");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write some text on screen with different sizes and color
    display.clearDisplay();
    for (int i = 0; i < 6; i++) {
      display.setTextColor(i);
      display.setTextSize(i +
                          1); // textSize parameter starts at 0 and goes up to
                              // 10 (larger won't fit Inkplate 4TEMPERA screen)
      display.setCursor(
          50, (i * i *
               8)); // setCursor works as same as on LCD displays - sets "the
                    // cursor" at the place you want to write someting next
      display.print("Inkplate4TEMPERA!"); // The actual text you want to show on
                                          // e-paper as String
    }
    displayCurrentAction(display, "Text in different sizes and shadings");
    display.display(); // To show stuff on screen, you always need to call
                       // display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write same text on different location, but now invert colors (text is
    // white, text background is black)
    display.setTextColor(7, 0); // First argument is text color, while second
                                // argument is background color. In greyscale,
    for (int i = 0; i < 6;
         i++) { // you are able to choose from 8 different colors (0-7)
      display.setTextSize(i + 1);
      display.setCursor(50, 300 + (i * i * 8));
      display.print("Inkplate 4TEMPERA!");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    display.setTextColor(0, 7); // reset text color to black

    // Draws an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.drawElipse(400, 200, 300, 300, 0);
    displayCurrentAction(display, "Drawing an elipse");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Fills an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.fillElipse(400, 200, 300, 300, 0);
    displayCurrentAction(display, "Drawing a filled elipse");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Code block for generating random points and sorting them in a counter
    // clockwise direction.
    int xt[10];
    int yt[10];
    int n = 10;
    for (int i = 0; i < n; ++i) {
      xt[i] = random(100, 500);
      yt[i] = random(100, 500);
    }
    int k;
    for (int i = 0; i < n - 1; ++i)
      for (int j = i + 1; j < n; ++j)
        if (atan2(yt[j] - 300, xt[j] - 400) < atan2(yt[i] - 300, xt[i] - 400)) {
          k = xt[i], xt[i] = xt[j], xt[j] = k;
          k = yt[i], yt[i] = yt[j], yt[j] = k;
        }

    // Draws a polygon, from x and y coordinate arrays of n points in color c
    display.clearDisplay();
    display.drawPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a polygon");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Fills a polygon, from x and y coordinate arrays of n points in color c,
    // Points need to be counter clockwise sorted
    // Method can be quite slow for now, probably will improve
    display.clearDisplay();
    display.fillPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a filled polygon");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write text and rotate it by 90 deg. forever
    int r = 0;
    display.setTextSize(5);
    display.setTextColor(7, 0);
    while (true) {
      display.setCursor(120, 250);
      display.clearDisplay();
      display.setRotation(
          r); // Set rotation will sent rotation for the entire display, so you
              // can use it sideways or upside-down
      display.print("Inkplate4TEMPERA");
      display.display();
      r++;
      vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    // Did you know that you can change between BW and greyscale mode anytime?
    // Just call display.selectDisplayMode(mode)
  }
}