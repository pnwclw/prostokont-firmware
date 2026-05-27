/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Download and display BMP images from the web (Inkplate 6 Color).
 *
 * @details     Demonstrates how to connect Inkplate 6 Color to a WiFi network,
 *              download a BMP image from a web URL (HTTP/HTTPS depending on
 *              the sketch implementation), and render it on the e-paper display
 *              using the Inkplate image drawing functions.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Color, USB cable
 * - Extra:      Stable WiFi connection
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6 Color
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 *
 * How to use:
 * 1) Set the image URL to a compatible BMP file.
 * 2) Build and flash to Inkplate 6 Color.
 * 3) The board connects to WiFi, downloads the image, and displays it.
 *
 * Expected output:
 * - BMP image downloaded from the web is displayed on the Inkplate screen.
 *
 * Notes:
 * - Supported BMP formats: Windows BMP, 1/4/8/24-bit color depth.
 * - Images must fit the display; large images may not render properly.
 * - Ensure the URL is directly pointing to the BMP file (no HTML redirect
 * pages).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

// link to image
#define IMAGE_PATH                                                             \
  "https://upload.wikimedia.org/wikipedia/commons/b/b5/"                       \
  "800x600_Wallpaper_Blue_Sky.png"

extern "C" void app_main(void) {
  Inkplate display;

  display.wifi.begin();
  display.wifi.waitForConnect();

  display.image.draw(IMAGE_PATH, 0, 0, true, false);
  display.display();
}