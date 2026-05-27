/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Calibrate the Inkplate 6 Color RTC by selecting crystal load
 *              capacitance and applying a clock offset, then display the time
 *              with periodic partial/full refreshes.
 *
 * @details     This example demonstrates how to improve RTC accuracy on
 *              Inkplate 6 Color by configuring the PCF85063(A) real-time clock.
 *              Two calibration mechanisms are shown:
 *
 *              1) Load capacitance selection:
 *                 Some boards populate external load capacitors for the 32.768
 * kHz crystal. If you choose to use the RTC's internal capacitor setting
 * instead, external capacitors must be removed. The sketch shows how to select
 * an internal capacitor value (e.g., 7 pF or 12.5 pF) using
 * setInternalCapacitor().
 *
 *              2) Clock offset correction:
 *                 The RTC supports a programmable offset (in ppm-equivalent
 *                 steps) applied periodically. setClockOffset(mode, value)
 *                 configures how often the correction is applied (mode) and the
 *                 signed correction magnitude (value).
 *
 *              After configuration, the sketch waits for a button press, sets
 *              an initial time, and then reads the RTC once per second and
 *              updates the e-paper display. To reduce flashing, it uses partial
 *              updates most of the time and forces a full refresh after a
 *              defined number of partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Color, USB cable
 * - Extra:      None.
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6 Color
 *
 * How to use:
 * 1) Decide whether you are using external crystal capacitors or the RTC's
 *    internal capacitor setting:
 *    - Using internal capacitor: remove external capacitors and enable
 *      setInternalCapacitor(...).
 *    - Using external capacitors: comment out setInternalCapacitor(...).
 * 2) (Optional) Determine and set the clock offset:
 *    - Best: measure the 32.768 kHz clock frequency and compute ppm deviation,
 *      then choose mode and offset register value accordingly.
 *    - Alternative: run without setClockOffset(), compare RTC time drift over
 *      2–3 days, estimate frequency error, then compute and apply an offset.
 * 3) Build and flash to Inkplate 6 Color.
 * 4) Press the wake button when prompted to start the RTC counter.
 * 5) Observe the displayed time; adjust capacitor/offset values if needed and
 *    re-upload.
 *
 * Expected output:
 * - E-paper: A prompt to press the wake button, then a large HH:MM:SS time that
 *   updates about once per second.
 *
 * Notes:
 * - Display mode is 1-bit (BW). Partial updates are supported only in BW mode.
 * - The displayed seconds may appear to “skip” or look uneven because e-paper
 *   refresh takes time; the RTC time itself continues accurately.
 * - Partial update best practice: do a full refresh every 5–10 partial updates
 *   to maintain image quality (this example enforces a threshold).
 * - partialUpdate(false, true) keeps the e-paper power enabled for faster
 *   successive updates (higher power usage).
 * - RTC offset parameters are hardware-specific; refer to the PCF85063(A)
 *   datasheet section on offset calibration for exact ppm/LSB behavior.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6 Color in the boards menu."
#endif

#include "driver/gpio.h"
#include "esp_timer.h"
#include "time.h"

#include "Inkplate.h"

#define REFRESH_DELAY 60000 // Delay between refreshes
unsigned long time1 = 0;    // Time for measuring refresh in millis

// Set initial time and date
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

// A function that adds 0 before one digit number
void print2Digits(Inkplate &display, uint8_t _d) {
  if (_d < 10)
    display.print('0');
  display.print(_d);
}

void printTime(Inkplate &display, uint8_t _hour, uint8_t _minutes,
               uint8_t _seconds) {
  // Print time
  print2Digits(display, _hour);
  display.print(':');
  print2Digits(display, _minutes);
  display.print(':');
  print2Digits(display, _seconds);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();               // Clear frame buffer of display
  display.setTextColor(INKPLATE_BLACK); // Set text color to black
  display.setTextSize(
      4); // Set text to be 4 times bigger than classic 5x7 px text

  gpio_set_direction(GPIO_NUM_36,
                     GPIO_MODE_INPUT); // Set wake-up button as input

  // Some Inkplates has external capacitors for RTC crystal, but you can use
  // internal one if you have issues with accuracy. IMPORTANT:
  //  - If you use an internal capacitor, you have to remove the external ones.
  //  - If you use an external one, you don't have the next line of code.
  // Here we setting internal capacitor value (7 pF):
  // display.rtc.setInternalCapacitor(0);
  // Another option is 12.5 pF:
  display.rtc.setInternalCapacitor(1);

  // Set offset for RTC crystal
  // The first argument is a mode (0 or 1):
  // 0 means that the offset is made once every two hours (Each LSB introduces
  // an offset of 4.34 ppm) 1 means that the offset is made every 4 minutes
  // (Each LSB introduces an offset of 4.069 ppm) The second argument is the
  // offset value in decimal (from -64 to 63). The real offset depends on the
  // mode and it is equal to the: offset in ppm for specific mode * offset value
  // in decimal. For example: mode 0 (4.34 ppm), offset value 15 = + 65.1 ppm
  // every 2 hours See 8.2.3 in the datasheet for more details
  display.rtc.setClockOffset(1, -63);

  // How to calculate this offset?
  // 1. Measure the frequency on the clock pin of the RTC (let's call it
  // fMeasured)
  // 2. Convert it to time (tMeasured = 1 / fMeasured)
  // 3. Calculate the difference to the ideal period of 1 / 32768.00: D = 1 /
  // 32768 - tMeasured
  // 4. Calculate the ppm deviation compared to the measured value: Eppm =
  // 1000000 * D / tMeasured
  // 5. Calculate the offset register value:
  // Mode = 0 -> Offset = Eppm / 4.34
  // Mode = 1 -> Offset = Eppm / 4.069
  // Round this number and this is the second parameter of the function

  // If you don't have an oscilloscope or something to measure the frequency,
  // here is a procedure for you. NOTE: This is a longer, but more precise
  // method to calibrate RTC. When you run for the first time to see how much
  // rtc misses, you MUST comment the display.rtc.setClockOffset() function
  // above. Once again, if you are using external capacitor, you don't need
  // neither display.rtc.setInternalCapacitor(); so also comment this line.

  // First, upload the code to the Inkplate.
  // It would be best if you had a clock on the side (on a phone or computer).
  // Press the wake button at a certain time.
  // Remember that time or write it down somewhere because you will need it for
  // the calculation. Let the Inkplate count for a while. The longer you wait,
  // the more accurate the calculation will be. We recommend waiting 2-3 days.
  // When that time has passed, remember the current time and the time displayed
  // on the Inkplate AT THE SAME TIME (We recommend taking pictures of them next
  // to each other). Now calculate how many seconds of real-time have passed and
  // how many the Inkplate has counted. Calculate the time counted by the
  // Inkplate divided by the time that actually passed. Then you will get a
  // number that needs to be multiplied by 32768. The number you get is the RTC
  // frequency. With that number, perform the above calculation as if you
  // measured the frequency with an oscilloscope. Pass the resulting number as
  // the second argument of the function.

  // Print a message for waiting
  display.println("RTC calibration");
  display.setTextColor(INKPLATE_RED);
  display.setTextSize(3);
  display.println("Press the wake-up button to start RTC");
  display.display();

  // Waiting for the button press
  while (gpio_get_level(GPIO_NUM_36) == 1) {
    // Waiting
    vTaskDelay(1);
  }

  struct tm time = {};
  time.tm_hour = 0;
  time.tm_sec = 0;
  time.tm_min = 0;

  // Set the RTC to begin
  display.rtc.setTime(time); // Send time to RTC

  while (true) {
    // Print new time every minutre
    if ((esp_timer_get_time() - time1) > REFRESH_DELAY) {
      seconds = display.rtc.getSecond(); // Store senconds in a variable
      minutes = display.rtc.getMinute(); // Store minutes in a variable
      hours = display.rtc.getHour();     // Store hours in a variable

      display.clearDisplay();     // Clear content in frame buffer
      display.setCursor(30, 200); // Set position of the text
      printTime(display, hours, minutes, seconds); // Print the time on screen

      time1 = esp_timer_get_time();

      display.display();
    }
  }

  return;
}