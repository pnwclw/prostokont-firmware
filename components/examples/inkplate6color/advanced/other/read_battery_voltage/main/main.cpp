/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Read and display battery voltage on Soldered Inkplate 6Color.
 *
 * @details     Continuously reads the LiPo battery voltage using the built-in
 *              ADC measurement and displays it on the e-paper screen along with
 *              a battery symbol bitmap. The reading refreshes every 10 seconds.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable, LiPo battery connected
 * - Extra:      battSymbol.h header with battery icon bitmap included in the
 * project
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Connect a LiPo battery to Inkplate 6Color.
 * 2) Build and flash to Inkplate 6Color.
 * 3) The display shows the current battery voltage, refreshing every 10
 * seconds.
 *
 * Expected output:
 * - Battery symbol with the measured voltage (e.g., "3.85V") displayed in blue.
 *
 * Notes:
 * - The voltage reading may not be accurate without a battery connected.
 * - display.display() triggers a full refresh every 10 seconds.
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
#include "battSymbol.h"

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
  Inkplate display;

  while (true) {
    float voltage = display.readBattery(); // Read battery voltage
    display
        .clearDisplay(); // Clear everything in frame buffer of e-paper display
    display.drawBitmap(
        100, 100, battSymbol, battSymbol_w, battSymbol_h,
        INKPLATE_BLUE); // Draw battery symbol at position X=100 Y=100
    display.setCursor(210, 120);
    display.setTextColor(INKPLATE_BLUE);
    display.setTextSize(3);
    display.print(voltage, 2); // Print battery voltage
    display.print('V');
    display.display(); // Send everything to display (refresh the screen)
    vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before new measurement
  }
}