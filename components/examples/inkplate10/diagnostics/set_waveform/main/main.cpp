/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Cycle through EPD waveforms on Inkplate 10 and display a
 *              grayscale gradient for each.
 *
 * @details     This example demonstrates how to test and compare different
 *              EPD waveform settings on the Inkplate 10. It iterates through
 *              waveforms 1–5, applying each one and rendering a full grayscale
 *              gradient pattern so you can visually evaluate contrast, banding,
 *              and ghosting for each waveform.
 *
 *              Use this to select the waveform that best suits your application
 *              before committing to one in production code.
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
 * 1) Build and flash to Inkplate 10.
 * 2) The display will cycle through waveforms 1–5 automatically,
 *    showing a grayscale gradient and the active waveform number for each.
 * 3) Observe the gradient quality for each waveform and note which
 *    looks best for your use case.
 * 4) Set your preferred waveform in your application using setWaveform().
 *
 * Expected output:
 * - E-paper: A grayscale gradient from black to white across 8 shades,
 *   with the active waveform number shown at the bottom. Cycles every 5
 * seconds.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (8 shades).
 * - Different waveforms trade off between refresh speed, ghosting, and
 *   gray level accuracy; there is no universally "best" choice.
 * - Waveform selection is not persisted; call setWaveform() at startup
 *   in your own application.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"
#include "image_ex.h"

static void show_gradient(Inkplate &inkplate, int selected) {
  inkplate.clearDisplay();

  int w = inkplate.width() / 8;
  int h = inkplate.height() - 100;

  inkplate.fillRect(0, 725, 1200, 100, 7);

  inkplate.setTextSize(4);
  inkplate.setTextColor(0);
  inkplate.setCursor(10, 743);

  inkplate.drawRect((selected * 6 * 4 * 2) + 432 - 3, 740, (6 * 4) + 2,
                    (8 * 4) + 2, 0);

  for (int i = 0; i < 8; i++)
    inkplate.fillRect(i * w, 0, w, h, i);

  inkplate.setTextSize(3);
  inkplate.setCursor(10, 792);
  inkplate.print("Waveform ");
  inkplate.print(selected);

  inkplate.display();
}

extern "C" void app_main(void) {
  Inkplate inkplate;

  for (int waveform = 1; waveform <= 5; waveform++) {
    printf("Testing waveform %d...\n", waveform);

    inkplate.setWaveform(waveform, false);
    show_gradient(inkplate, waveform);

    vTaskDelay(pdMS_TO_TICKS(5000)); // 3 seconds per waveform
  }
}