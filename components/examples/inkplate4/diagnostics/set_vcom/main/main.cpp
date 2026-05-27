/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Set and verify VCOM voltage for Soldered Inkplate 4TEMPERA.
 *
 * @details     Programs the e-paper panel VCOM voltage to the value defined by
 *              VCOM_VALUE and displays a grayscale test image so the result can
 * be visually verified. The stored VCOM value is also printed on screen. VCOM
 * affects image contrast and uniformity; the correct value is printed on the
 * e-paper panel's FPC cable.
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
 * 1) Set VCOM_VALUE to the value printed on your panel's FPC cable (e.g.,
 * -2.6). 2) Build and flash to Inkplate 4TEMPERA. 3) The VCOM is programmed and
 * a grayscale test image is shown.
 *
 * Expected output:
 * - "Stored VCOM: X.XX V" and 8 grayscale bars displayed on screen.
 *
 * Notes:
 * - VCOM is stored in non-volatile memory and only needs to be set once.
 * - An incorrect VCOM value can cause poor contrast or uneven display updates.
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

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

#define VCOM_VALUE (-2.6)

static void display_test_image(Inkplate &inkplate) {
  inkplate.clearDisplay();

  double vcom = inkplate.getStoredVCOM();

  inkplate.setTextColor(0);
  inkplate.setTextSize(2);
  inkplate.setCursor(5, 5);
  inkplate.print("Stored VCOM: ");
  inkplate.print(vcom);
  inkplate.print(" V");

  int w = inkplate.width() / 8;
  int h = inkplate.height();

  for (int i = 0; i < 8; i++) {
    int x = w * i;
    inkplate.fillRect(x, 40, w, h, i);
  }

  inkplate.display();
}

extern "C" void app_main(void) {
  Inkplate inkplate;

  printf("Setting VCOM to %.2f\n", VCOM_VALUE);

  if (inkplate.setVCOM(VCOM_VALUE) == ESP_OK)
    printf("VCOM programmed OK\n");
  else
    printf("VCOM programming failed\n");

  display_test_image(inkplate);
}