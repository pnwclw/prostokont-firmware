/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Deep sleep slideshow example for Soldered Inkplate 6Color.
 *
 * @details     Demonstrates deep sleep power saving by cycling through three
 *              pre-loaded images stored in flash. On each wake-up the next
 *              image is displayed, then the board enters deep sleep for 20
 *              seconds using the ESP32 timer wakeup. The current slide index
 *              is preserved across sleep cycles using RTC memory.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      picture1.h, picture2.h, picture3.h headers with 600x448 image
 * arrays
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Add your three 600x448 images as picture1.h, picture2.h, picture3.h in the
 * main/ folder. 2) Build and flash to Inkplate 6Color. 3) The display cycles
 * through images, sleeping 20 seconds between each.
 *
 * Expected output:
 * - Images displayed one by one, with the board waking from deep sleep to
 * advance the slideshow.
 *
 * Notes:
 * - RTC_DATA_ATTR keeps the slide counter in RTC memory across deep sleep
 * cycles.
 * - Deep sleep resets normal RAM, so only RTC-backed variables survive.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6Color in the boards menu."
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
static const uint8_t *pictures[] = {picture1, picture2, picture3};

RTC_DATA_ATTR static int slide = 0;

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay(); // Clear frame buffer of display
  // Display selected picture at location X=0, Y=0.
  display.image.draw(pictures[slide], 0, 0, 600, 448,
                     0); // Display selected picture at location X=0, Y=0. All
                         // three pictures have resolution of 600x448 pixels
  display.display();     // Refresh the screen with new picture

  slide++;
  if (slide > 2)
    slide = 0; // We do not have more than 3 images, so roll back to zero

  // Activate wake-up timer -- wake up after 20s here
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}