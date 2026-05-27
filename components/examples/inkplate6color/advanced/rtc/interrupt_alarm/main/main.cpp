/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm via GPIO interrupt for Soldered Inkplate 6Color.
 *
 * @details     Configures the RTC to raise a falling-edge interrupt on GPIO 39
 *              when the alarm fires. A GPIO ISR sets a flag that is checked in
 *              the main loop. The current time (set via epoch) is displayed on
 *              screen and updated every 30 seconds. When the alarm fires 80
 *              seconds after start, "ALARM" is shown on screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6Color
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6Color, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Display Board -> Inkplate6Color
 *
 * How to use:
 * 1) Build and flash to Inkplate 6Color.
 * 2) The display shows the running time; "ALARM" appears after ~80 seconds.
 *
 * Expected output:
 * - Current time displayed in red, updating every 30 seconds.
 * - "ALARM" printed when the RTC interrupt triggers.
 *
 * Notes:
 * - The RTC interrupt pin is GPIO 39 (RTC_INT_PIN).
 * - The alarm is set 80 seconds after the hardcoded epoch 1589610300.
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
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_INT_ALARM";

#define REFRESH_DELAY_MS 30000
#define RTC_INT_PIN GPIO_NUM_39

static volatile int s_alarmFlag = 0;

static void IRAM_ATTR alarmISR(void *arg) { s_alarmFlag = 1; }

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

static void printTime(Inkplate &display, uint8_t hour, uint8_t minutes,
                      uint8_t seconds, uint8_t day, uint8_t weekday,
                      uint8_t month, uint16_t year) {
  const char *wday[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};

  print2Digits(display, hour);
  display.print(':');
  print2Digits(display, minutes);
  display.print(':');
  print2Digits(display, seconds);
  display.print(' ');
  display.print(wday[weekday % 7]);
  display.print(", ");
  print2Digits(display, day);
  display.print('/');
  print2Digits(display, month);
  display.print('/');
  display.print(year);
}

extern "C" void app_main(void) {
  Inkplate display;

  // Configure RTC interrupt pin
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << RTC_INT_PIN);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.intr_type = GPIO_INTR_NEGEDGE; // FALLING edge like attachInterrupt
  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(RTC_INT_PIN, alarmISR, NULL);

  display.clearDisplay(); // Clear frame buffer of display
  display.setTextSize(3);
  display.setCursor(60, 100); // Set position of the text
  display.setTextColor(INKPLATE_RED,
                       INKPLATE_WHITE); // Set text color and background

  // Set time via epoch (1589610300 = 2020-05-16 06:25:00 UTC)
  esp_err_t err = display.rtc.setTime((time_t)1589610300);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  // Set alarm 10 seconds later
  err = display.rtc.setAlarmEpoch((time_t)(1589610300 + 80));
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setAlarmEpoch failed: %s", esp_err_to_name(err));

  int n = 0;

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    display.clearDisplay();
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday,
              now.tm_mon,   // driver returns 1-based
              now.tm_year); // driver returns full year e.g. 2020

    if (s_alarmFlag) {
      display.rtc.clearAlarmFlag();
      display.setCursor(200, 200);
      display.print("ALARM");
      ESP_LOGI(TAG, "Alarm triggered!");
    }

    display.display();

    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}