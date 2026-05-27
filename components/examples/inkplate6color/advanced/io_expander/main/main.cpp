/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       IO expander GPIO blink example for Soldered Inkplate 6Color.
 *
 * @details     Demonstrates controlling an external GPIO through the on-board
 *              PCAL IO expander. Pin P1-7 (GPB7) is configured as an output
 *              and toggled five times with 500 ms intervals, then pauses for
 *              2 seconds before repeating. An LED (or other load) connected
 *              to that pin will blink accordingly.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      LED or other load connected to IO expander pin GPB7
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Connect an LED (with a resistor) to IO expander pin GPB7.
 * 2) Build and flash to Inkplate 6Color.
 * 3) The LED blinks five times, pauses, then repeats indefinitely.
 *
 * Expected output:
 * - LED on GPB7 blinks five times with 500 ms on/off, then a 2-second pause.
 *
 * Notes:
 * - expander1 is the internal IO expander declared as an extern in Inkplate.h.
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
#include "PCAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// We are going to use pin P1-7 (GPB7) = IO_NUM_B7
#define LED_PIN IO_NUM_B7

// expander1 is the internal IO expander (addr IO_INT_ADDR = 0x20), declared
// as an extern in Inkplate.h — it is owned by the board driver.
extern PCAL expander1;

extern "C" void app_main(void) {
  Inkplate display;

  // Configure LED pin as output on IO expander.
  expander1.setDirection(LED_PIN, IO_MODE_OUTPUT);

  while (true) {
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