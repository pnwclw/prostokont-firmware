/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC crystal calibration utility for Soldered Inkplate 4TEMPERA.
 *
 * @details     Provides a step-by-step procedure for calibrating the RTC
 * crystal oscillator. After pressing the wake-up button, the RTC is zeroed and
 * the elapsed time is displayed every second, alternating between partial and
 * full updates. The internal capacitor and clock offset registers can be
 * adjusted to correct drift. Detailed calculation instructions are provided in
 * the source code comments.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Optional: oscilloscope for measuring RTC crystal frequency
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate4
 *
 * How to use:
 * 1) Optionally comment out setClockOffset() and setInternalCapacitor() for the
 *    first run to measure the raw drift.
 * 2) Build and flash to Inkplate 4TEMPERA.
 * 3) Press the wake-up button to start the timer.
 * 4) After 2-3 days, compare the displayed time to actual time and calculate
 *    the offset as described in the source comments.
 * 5) Set the calculated offset and rebuild.
 *
 * Expected output:
 * - “Press wake button to start RTC!” on screen; HH:MM:SS displayed after
 * button press, updating every second.
 *
 * Notes:
 * - The wake-up button is connected to GPIO 36.
 * - A full refresh is forced every MAX_PARTIAL_UPDATES partial updates.
 * - See PCF85063A datasheet section 8.2.3 for offset register details.
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

#include "driver/gpio.h"
#include "esp_timer.h"
#include "time.h"

#include "Inkplate.h"

#define REFRESH_DELAY 1000 // Delay between refreshes
unsigned long time1 = 0;   // Time for measuring refresh in millis

#define MAX_PARTIAL_UPDATES                                                    \
  9 // How many partial updates to do before a full refresh

// Variable that keeps count on how much screen has been partially updated
int n = 0;

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

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display
  display.setTextSize(
      3); // Set text to be 5 times bigger than classic 5x7 px text
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
  display.setCursor(160, 280);
  display.println("Press wake button");
  display.setCursor(210, 315);
  display.println("to start RTC!");
  display.partialUpdate();

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
    // Print new time every second
    // NOTE: The display needs some time to refresh, so the time will
    // sometimes seem wrong but that actual RTC time will be precise
    if ((esp_timer_get_time() - time1) > REFRESH_DELAY) {
      seconds = display.rtc.getSecond(); // Store senconds in a variable
      minutes = display.rtc.getMinute(); // Store minutes in a variable
      hours = display.rtc.getHour();     // Store hours in a variable

      display.clearDisplay();      // Clear content in frame buffer
      display.setCursor(190, 290); // Set position of the text
      printTime(display, hours, minutes, seconds); // Print the time on screen

      if (n > MAX_PARTIAL_UPDATES) // Check if you need to do full refresh or
                                   // you can do partial update
      {
        display.display(true); // Do a full refresh
        n = 0;
      } else {
        display.partialUpdate(
            false, true); // Do partial update and keep e-papr power supply on
        n++; // Keep track on how many times screen has been partially updated
      }

      time1 = esp_timer_get_time();
    }
  }

  return;
}