/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Basic GPIO control example: blink an external LED using
 *              ESP32 GPIO on Inkplate 2.
 *
 * @details     This example demonstrates how to use general-purpose I/O (GPIO)
 *              pins available on the Inkplate 2 header. An external LED is
 *              connected to ESP32 GPIO14 through a current-limiting resistor.
 *              The sketch configures the selected pin as an OUTPUT and toggles
 *              it every second, creating a visible blink.
 *
 *              The e-paper display is used only to show instructions and runs
 *              in 1-bit (black/white) mode with a single full refresh during
 *              setup.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      LED, ~330 Ω resistor, jumper wires, breadboard
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate2
 * - Connect LED anode to GPIO14 through a 330 Ω resistor
 * - Connect LED cathode to GND
 *
 * How to use:
 * 1) Wire the LED to GPIO14 with a current-limiting resistor.
 * 2) Build and flash to Inkplate 2.
 * 3) After reset, read instructions on the display.
 * 4) Observe the LED blinking once per second (1 s ON, 1 s OFF).
 *
 * Expected output:
 * - Display: "Blink example" and wiring instructions.
 * - Hardware: LED connected to GPIO14 blinks continuously.
 *
 * Notes:
 * - Use only GPIO pins that are exposed on the Inkplate 2 header and not
 *   reserved for internal hardware.
 * - Ensure correct polarity of the LED and always use a resistor to avoid
 *   damaging the GPIO pin.
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
#include "driver/gpio.h"

#define PIN_LED GPIO_NUM_14

extern "C" void app_main(void) {
  Inkplate display;

  display.setTextSize(1);    // Set text size
  display.setCursor(10, 20); // Set cursor position
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);
  display.println("Blink example");
  display.setCursor(10, 35); // Set cursor position
  display.println(
      "Connect LED to ESP32 GPIO14 and LED will blink once every two seconds.");

  // Display to screen
  display.display();

  // Set LED GPIO to be output pin
  gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

  while (true) {
    gpio_set_level(PIN_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(PIN_LED, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}