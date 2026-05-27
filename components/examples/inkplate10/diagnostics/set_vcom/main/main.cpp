/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads and programs the Inkplate 10 EPD VCOM voltage via Serial.
 *
 * @details     This example shows how to read the currently stored VCOM value
 *              from the display power IC/EEPROM and optionally program a new
 *              VCOM value. After programming, a simple grayscale test pattern
 *              is drawn and the stored VCOM value is shown on the e-paper
 * display.
 *
 *              VCOM is stored in EEPROM and can only be programmed a limited
 *              number of times. Do NOT run this sketch repeatedly or "tune"
 *              VCOM by trial-and-error. Program it once (only if needed) and
 *              leave it unchanged to avoid prematurely wearing out EEPROM.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      None.
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Set the desired VCOM value in the VCOM_VALUE define.
 * 2) Build and flash to Inkplate 10.
 * 3) The programmed VCOM value will be shown on the display alongside
 *    a grayscale test pattern to verify display quality.
 *
 * Expected output:
 * - E-paper: The stored VCOM voltage value and a grayscale gradient test
 * pattern.
 *
 * Notes:
 * - VCOM should typically be set once during manufacturing or initial setup.
 * - Incorrect VCOM values can cause poor contrast or display artifacts.
 * - Refer to your panel's datasheet for the correct VCOM value.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

#define VCOM_VALUE (-3.0)

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

  if (inkplate.setVCOM(VCOM_VALUE))
    printf("VCOM programmed OK\n");
  else
    printf("VCOM programming failed\n");

  display_test_image(inkplate);
}