/**
 * @file Waveshare13.h
 * @brief Board driver for Waveshare 13.3 inch E6 Spectra panel.
 */

#pragma once

#include "PanelCommon.h"
#include "GraphicsDefs.h"
#include "SPI.h"
#include "driver/gpio.h"

// Waveshare 13.3 inch E6 Spectra color indexes.
#define WAVESHARE13_BLACK 0
#define WAVESHARE13_WHITE 1
#define WAVESHARE13_YELLOW 2
#define WAVESHARE13_RED 3
#define WAVESHARE13_BLUE 5
#define WAVESHARE13_GREEN 6

// Internal framebuffer orientation for the 13.3 inch E6 Spectra panel.
#define E_INK_WIDTH 1200
#define E_INK_HEIGHT 1600

// Waveshare Frame board pinout.
#define WAVESHARE13_SCLK_PIN GPIO_NUM_9
#define WAVESHARE13_MOSI_PIN GPIO_NUM_46
#define WAVESHARE13_CS_M_PIN GPIO_NUM_10
#define WAVESHARE13_CS_S_PIN GPIO_NUM_3
#define WAVESHARE13_DC_PIN GPIO_NUM_11
#define WAVESHARE13_RST_PIN GPIO_NUM_2
#define WAVESHARE13_BUSY_PIN GPIO_NUM_12
#define WAVESHARE13_PWR_PIN GPIO_NUM_1

// Register addresses.
#define WAVESHARE13_REGISTER_PSR 0x00
#define WAVESHARE13_REGISTER_PWR 0x01
#define WAVESHARE13_REGISTER_POF 0x02
#define WAVESHARE13_REGISTER_PON 0x04
#define WAVESHARE13_REGISTER_BTST_N 0x05
#define WAVESHARE13_REGISTER_BTST_P 0x06
#define WAVESHARE13_REGISTER_DTM 0x10
#define WAVESHARE13_REGISTER_DRF 0x12
#define WAVESHARE13_REGISTER_PLL 0x30
#define WAVESHARE13_REGISTER_CDI 0x50
#define WAVESHARE13_REGISTER_TCON 0x60
#define WAVESHARE13_REGISTER_TRES 0x61
#define WAVESHARE13_REGISTER_AN_TM 0x74
#define WAVESHARE13_REGISTER_AGID 0x86
#define WAVESHARE13_REGISTER_BUCK_BOOST_VDDN 0xB0
#define WAVESHARE13_REGISTER_TFT_VCOM_POWER 0xB1
#define WAVESHARE13_REGISTER_EN_BUF 0xB6
#define WAVESHARE13_REGISTER_BOOST_VDDP_EN 0xB7
#define WAVESHARE13_REGISTER_CCSET 0xE0
#define WAVESHARE13_REGISTER_PWS 0xE3
#define WAVESHARE13_REGISTER_CMD66 0xF0

class Waveshare13 : public PanelCommon {
public:
  Waveshare13();

  uint32_t partialUpdate(bool = false, bool = false) { return 0; }
  esp_err_t einkOn() { return setPanelDeepSleep(false) ? ESP_OK : ESP_FAIL; }
  esp_err_t einkOff() { return setPanelDeepSleep(true) ? ESP_OK : ESP_FAIL; }

private:
  enum eSpectraChipID {
    eChipIdMaster = 1,
    eChipIdSlave = 2,
    eChipIdBoth = eChipIdMaster | eChipIdSlave
  };

  esp_err_t initBuffers();
  esp_err_t display3b(bool leaveOn);

  bool waitForEpd(uint32_t timeout);
  void resetPanel();
  bool setPanelDeepSleep(bool sleep);
  void setPanelPinsToLow();
  void screenInit();
  void sendCommandData(uint8_t cmd, const uint8_t *data, int n,
                       enum eSpectraChipID chipId);
  void turnOnDisplay(bool leaveOn);

  void calculateLUTs() {}
  esp_err_t display1b(bool) { return ESP_OK; }
  void gpioInit() {}
  void clean(uint8_t, uint8_t) {}
  void pinsAsOutputs() {}
  void pinsZstate() {}

  SPI m_spi;
};
