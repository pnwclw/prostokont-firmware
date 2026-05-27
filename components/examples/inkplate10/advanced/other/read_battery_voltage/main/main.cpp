/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Battery voltage reading example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to read the connected Li-ion/Li-Po battery
 *              voltage using Inkplate’s built-in battery measurement circuitry.
 *              The example shows how to obtain the battery voltage value in
 *              software and display or process it as needed.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable, 3.6–4.2 V Li-ion/Li-Po battery (JST
 * connector)
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate10
 *
 * How to use:
 * 1) Connect a supported Li-ion/Li-Po battery to the Inkplate battery
 * connector. 2) Build and flash to Inkplate 10. 3) The battery voltage is read
 * and can be displayed or logged by the sketch.
 *
 * Expected output:
 * - Measured battery voltage value reported by the program.
 *
 * Notes:
 * - Battery voltage reading is enabled through the onboard circuitry.
 * - Accuracy depends on battery condition and load.
 * - Battery reading typically requires enabling the battery measurement path
 *   in hardware (see Inkplate documentation).
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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"
#include "battSymbol.h"

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);

  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display
  display.setTextSize(
      2); // Scale text to be two times bigger then original (5x7 px)
  display.setTextColor(
      BLACK, WHITE); // Set text color to black and background color to white

  while (true) {
    float voltage = display.readBattery(); // Read battery voltage
    display.clearDisplay();                // Clear frame buffer of display
    display.image.draw(battSymbol, 100, 100, 106, 45,
                       BLACK); // Draw battery symbol at position X=100 Y=100
    display.setCursor(210, 120);
    display.print(voltage, 2); // Print voltage
    display.print('V');
    display.display(); // Send everything to display (refresh the screen)
    vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before new measurement
  }
}