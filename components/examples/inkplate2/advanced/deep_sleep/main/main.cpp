/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple deep sleep slideshow example for Soldered Inkplate 2.
 *
 * @details     Demonstrates low-power operation on Inkplate 2 using ESP32 deep
 *              sleep. On each wake-up (timer-based), the board redraws the
 * screen with the next image in a small slideshow, performs a full display
 *              refresh, and then returns to deep sleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      Converted image header files (picture1.h, picture2.h,
 * picture3.h)
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 3) The board will show an image, go to deep sleep, and wake up every 20
 * seconds. 4) After each wake-up, the next image is shown (loops through 3
 * images).
 *
 * Expected output:
 * - Inkplate display shows a new image every 20 seconds.
 * - The slideshow loops through all provided images.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - RAM contents are lost during deep sleep, so standard partial updates cannot
 * be used.
 * - This example uses 3-bit (grayscale) mode, which requires full refresh
 * updates.
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
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "picture1.h"
#include "picture2.h"
#include "picture3.h"

#define TIME_TO_SLEEP_US (20ULL * 1000000ULL)

// This array of pinters holds address of every picture in the memory,
// so we can easly select it by selecting index in array
static const uint8_t *pictures[] = {pic1, pic2, pic3};
const uint8_t w[] = {159, 148, 186}; // Widths of each picture. Heights are the
                                     // same for oll three pictures.

RTC_DATA_ATTR static int slide = 0;

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay(); // Clear frame buffer of display
  // Display selected picture at location X=0, Y=0.
  display.image.draw(pictures[slide], 106 - w[slide] / 2, 0, w[slide], 104,
                     BLACK);
  display.display(); // Refresh the screen with new picture

  slide++;
  if (slide > 2)
    slide = 0; // We do not have more than 3 images, so roll back to zero

  // Activate wake-up timer -- wake up after 20s here
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}