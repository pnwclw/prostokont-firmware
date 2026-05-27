/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Raw touchscreen register dump for Soldered Inkplate 4TEMPERA.
 *
 * @details     Reads the 8 raw touchscreen controller registers each time a
 * touch is detected and prints them bit-by-bit to the serial output. Useful for
 * debugging touch hardware or understanding the low-level register layout of
 * the touchscreen controller. Also draws a reference triangle at the (0,0)
 * corner of the display.
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
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Open a serial monitor at the default baud rate.
 * 3) Touch the screen to print the register values.
 *
 * Expected output:
 * - 8 lines of 8-bit binary register values printed to serial on each touch
 * event.
 *
 * Notes:
 * - getRawData() fills an 8-byte array with the raw register contents.
 * - Registers are printed once per second when a touch is detected.
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

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint8_t touchRegs[8];

extern "C" void app_main() {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();

  display.fillTriangle(13, 13, 23, 43, 43, 23, BLACK);
  display.setTextSize(3);
  display.setCursor(65, 65);
  display.print("(0,0) position");
  display.display();

  while (true) {
    if (display.touchscreen.available()) {
      display.touchscreen.getRawData(touchRegs);
      for (int i = 0; i < 8; ++i) {
        printf("Reg [%d]: ", i);
        for (int bit = 7; bit >= 0; --bit) {
          printf("%d", (touchRegs[i] >> bit) & 1);
        }
        printf("\n");
      }
      printf("---------------------------\n\n");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}