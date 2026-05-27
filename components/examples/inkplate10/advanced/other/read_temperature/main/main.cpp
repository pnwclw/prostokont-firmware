/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       On-board temperature sensor reading example for Soldered
 * Inkplate 10.
 *
 * @details     Demonstrates how to read temperature data from the on-board
 *              temperature sensor integrated inside the TPS65186 e-paper PMIC.
 *              This sensor is intended primarily for internal compensation
 *              and basic monitoring. It is a simple (basic) temperature sensor
 *              and should not be considered highly accurate or suitable for
 *              precise temperature measurements.
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
 * 1) Build and flash to Inkplate 10.
 * 2) The program reads the temperature from the onboard PMIC sensor.
 * 3) The measured value can be displayed or printed to Serial.
 *
 * Expected output:
 * - Approximate temperature reading reported by the TPS65186 sensor.
 *
 * Notes:
 * - The TPS65186 PMIC includes a basic internal temperature sensor.
 * - This sensor is not ultra-precise and is not a replacement for a
 *   dedicated external temperature sensor.
 * - Intended use is system monitoring and waveform compensation.
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
#include "tempSymbol.h"

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
    int temperature = display.readTemperature(); // Read temperature from
                                                 // on-board temperature sensor
    display.clearDisplay(); // Clear frame buffer of display
    display.image.draw(
        tempSymbol, 100, 100, 38, 79,
        BLACK); // Draw temperature symbol at position X=100, Y=100
    display.setCursor(150, 125);
    display.print(temperature); // Print temperature
    display.print('C');
    display.display(); // Send everything to display (refresh the screen)
    vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before new measurement
  }
}