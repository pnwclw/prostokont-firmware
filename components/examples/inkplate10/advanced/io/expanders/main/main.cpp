/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Internal and external IO expander control example for Soldered
 * Inkplate 10.
 *
 * @details     Demonstrates how to control GPIO pins on both the internal and
 *              external IO expanders available on Inkplate 10. The example
 *              alternates blinking an LED connected to the external IO expander
 *              (IO Expander 2) and an LED connected to the internal IO expander
 *              (IO Expander 1), showing correct usage and addressing for each.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Connect an LED + 330 Ω resistor to P1-7 (GPB7) on IO Expander 2
 * (external). 2) Connect another LED + 330 Ω resistor to P1-7 (GPB7) on IO
 * Expander 1 (internal). 3) Build and flash to Inkplate 10. 4) Observe
 * alternating blinking between external and internal LEDs.
 *
 * Expected output:
 * - External IO expander LED blinks for 5 seconds.
 * - Internal IO expander LED blinks for 5 seconds.
 * - Sequence repeats continuously.
 *
 * Notes:
 * - External IO expander pins are all free to use by default.
 * - Internal IO expander has restrictions:
 *   - DO NOT use GPA0–GPA7 or GPB0.
 *   - Use only pins 9–15 (P1-1 to P1-7).
 * - Using restricted pins may permanently damage the display.
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

#include "Inkplate.h"
#include "PCAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// We are going to use pin P1-7 (GPB7) = IO_NUM_B7 on both expanders.
// GPA0 = IO_NUM_A0 ... GPA7 = IO_NUM_A7
// GPB0 = IO_NUM_B0 ... GPB7 = IO_NUM_B7
#define LED_PIN IO_NUM_B7

// expander1 is the internal IO expander (addr IO_INT_ADDR = 0x20), declared
// as an extern in Inkplate10.h — it is owned by the board driver.
extern PCAL expander1;

// expander2 is the external IO expander (addr IO_EXT_ADDR = 0x21).
// Declare it here the same way expander1 is declared in the library.
extern PCAL expander2;

extern "C" void app_main(void) {
  Inkplate display;

  // Configure LED pin as output on both IO expanders.
  expander2.setDirection(LED_PIN, IO_MODE_OUTPUT);
  expander1.setDirection(LED_PIN, IO_MODE_OUTPUT);

  while (true) {
    // External IO Expander (IO Expander 2)
    for (int i = 0; i < 5; i++) {
      expander2.setLevel(LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      expander2.setLevel(LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Internal IO Expander (IO Expander 1)
    for (int i = 0; i < 5; i++) {
      expander1.setLevel(LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      expander1.setLevel(LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}