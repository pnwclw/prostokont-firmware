/**
 * @file Waveshare13.cpp
 * @brief Board driver for Waveshare 13.3 inch E6 Spectra panel.
 */

#include "panels/waveshare/Waveshare13.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#ifndef CONFIG_SPIRAM
#error "Waveshare 13.3 inch E6 requires external PSRAM for the 960 KB framebuffer."
#endif

static const char *TAG = "WAVESHARE13";

static const uint8_t PSR_V[2] = {0xDF, 0x6B};
static const uint8_t PWR_V[6] = {0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38};
static const uint8_t POF_V[1] = {0x00};
static const uint8_t DRF_V[1] = {0x00};
static const uint8_t PLL_V[1] = {0x08};
static const uint8_t CDI_V[1] = {0xF7};
static const uint8_t TCON_V[2] = {0x03, 0x03};
static const uint8_t TRES_V[4] = {0x04, 0xB0, 0x03, 0x20};
static const uint8_t CMD66_V[6] = {0x49, 0x55, 0x13, 0x5D, 0x05, 0x10};
static const uint8_t EN_BUF_V[1] = {0x07};
static const uint8_t CCSET_V[1] = {0x01};
static const uint8_t PWS_V[1] = {0x22};
static const uint8_t AN_TM_V[9] = {0xC0, 0x1C, 0x1C, 0xCC, 0xCC,
                                   0xCC, 0x15, 0x15, 0x55};
static const uint8_t AGID_V[1] = {0x10};
static const uint8_t BTST_P_V[2] = {0xD8, 0x18};
static const uint8_t BOOST_VDDP_EN_V[1] = {0x01};
static const uint8_t BTST_N_V[2] = {0xD8, 0x18};
static const uint8_t BUCK_BOOST_VDDN_V[1] = {0x01};
static const uint8_t TFT_VCOM_POWER_V[1] = {0x02};

Waveshare13::Waveshare13()
    : PanelCommon(E_INK_WIDTH, E_INK_HEIGHT, 0, 0),
      m_spi(WAVESHARE13_MOSI_PIN, WAVESHARE13_SCLK_PIN) {
  ESP_ERROR_CHECK(initBuffers());
  clearDisplay();

  gpio_set_direction(WAVESHARE13_RST_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(WAVESHARE13_DC_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(WAVESHARE13_CS_M_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(WAVESHARE13_CS_S_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(WAVESHARE13_PWR_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(WAVESHARE13_BUSY_PIN, GPIO_MODE_INPUT);

  gpio_pullup_dis(WAVESHARE13_BUSY_PIN);
  setPanelPinsToLow();

  ESP_LOGI(TAG, "Initialization finished");
}

esp_err_t Waveshare13::initBuffers() {
  m_framebufferColor = (uint8_t *)heap_caps_malloc(
      E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!m_framebufferColor)
    return ESP_ERR_NO_MEM;

  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);
  return ESP_OK;
}

esp_err_t Waveshare13::display3b(bool leaveOn) {
  if (!setPanelDeepSleep(false))
    return ESP_FAIL;

  gpio_set_level(WAVESHARE13_CS_M_PIN, 0);
  gpio_set_level(WAVESHARE13_CS_S_PIN, 1);
  m_spi.sendCommand(WAVESHARE13_REGISTER_DTM, WAVESHARE13_DC_PIN);
  for (int row = 0; row < E_INK_HEIGHT; row++) {
    m_spi.sendData(m_framebufferColor + (row * E_INK_WIDTH / 2),
                   E_INK_WIDTH / 4, WAVESHARE13_DC_PIN);
  }
  gpio_set_level(WAVESHARE13_CS_M_PIN, 1);

  if (!waitForEpd(60000))
    return ESP_FAIL;

  gpio_set_level(WAVESHARE13_CS_S_PIN, 0);
  gpio_set_level(WAVESHARE13_CS_M_PIN, 1);
  m_spi.sendCommand(WAVESHARE13_REGISTER_DTM, WAVESHARE13_DC_PIN);
  for (int row = 0; row < E_INK_HEIGHT; row++) {
    m_spi.sendData(m_framebufferColor + (row * E_INK_WIDTH / 2) +
                       (E_INK_WIDTH / 4),
                   E_INK_WIDTH / 4, WAVESHARE13_DC_PIN);
  }
  gpio_set_level(WAVESHARE13_CS_S_PIN, 1);

  turnOnDisplay(leaveOn);
  return ESP_OK;
}

bool Waveshare13::waitForEpd(uint32_t timeout) {
  uint32_t elapsed = 0;
  const uint32_t step = 10;

  while (gpio_get_level(WAVESHARE13_BUSY_PIN) == 0) {
    if (elapsed >= timeout) {
      ESP_LOGE(TAG, "EPD busy timeout");
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(step));
    elapsed += step;
  }

  return true;
}

void Waveshare13::resetPanel() {
  gpio_set_level(WAVESHARE13_RST_PIN, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(WAVESHARE13_RST_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
}

bool Waveshare13::setPanelDeepSleep(bool sleep) {
  if (!sleep) {
    if (!m_spi.isInitialized())
      m_spi.init();

    setPanelPinsToLow();
    vTaskDelay(pdMS_TO_TICKS(10));

    gpio_set_level(WAVESHARE13_DC_PIN, 1);
    gpio_set_level(WAVESHARE13_CS_M_PIN, 1);
    gpio_set_level(WAVESHARE13_CS_S_PIN, 1);
    gpio_set_level(WAVESHARE13_PWR_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    resetPanel();
    if (!waitForEpd(60000))
      return false;

    screenInit();
    return true;
  }

  if (m_spi.isInitialized()) {
    const uint8_t deep_sleep_check = 0xA5;
    sendCommandData(0x07, &deep_sleep_check, sizeof(deep_sleep_check),
                    eChipIdBoth);
  }

  vTaskDelay(pdMS_TO_TICKS(100));
  setPanelPinsToLow();
  gpio_set_direction(WAVESHARE13_DC_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(WAVESHARE13_CS_M_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(WAVESHARE13_CS_S_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(WAVESHARE13_RST_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(WAVESHARE13_BUSY_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(WAVESHARE13_PWR_PIN, GPIO_MODE_INPUT);

  if (m_spi.isInitialized())
    m_spi.deinit();

  return true;
}

void Waveshare13::setPanelPinsToLow() {
  const gpio_num_t pins[] = {WAVESHARE13_DC_PIN, WAVESHARE13_CS_M_PIN,
                             WAVESHARE13_CS_S_PIN, WAVESHARE13_RST_PIN,
                             WAVESHARE13_PWR_PIN};
  for (auto pin : pins) {
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
  }
}

void Waveshare13::screenInit() {
  sendCommandData(WAVESHARE13_REGISTER_AN_TM, AN_TM_V, sizeof(AN_TM_V),
                  eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_CMD66, CMD66_V, sizeof(CMD66_V),
                  eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_PSR, PSR_V, sizeof(PSR_V), eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_PLL, PLL_V, sizeof(PLL_V), eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_CDI, CDI_V, sizeof(CDI_V), eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_TCON, TCON_V, sizeof(TCON_V),
                  eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_AGID, AGID_V, sizeof(AGID_V),
                  eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_PWS, PWS_V, sizeof(PWS_V), eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_CCSET, CCSET_V, sizeof(CCSET_V),
                  eChipIdBoth);
  sendCommandData(WAVESHARE13_REGISTER_TRES, TRES_V, sizeof(TRES_V),
                  eChipIdBoth);

  sendCommandData(WAVESHARE13_REGISTER_PWR, PWR_V, sizeof(PWR_V),
                  eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_EN_BUF, EN_BUF_V, sizeof(EN_BUF_V),
                  eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_BTST_P, BTST_P_V, sizeof(BTST_P_V),
                  eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_BOOST_VDDP_EN, BOOST_VDDP_EN_V,
                  sizeof(BOOST_VDDP_EN_V), eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_BTST_N, BTST_N_V, sizeof(BTST_N_V),
                  eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V,
                  sizeof(BUCK_BOOST_VDDN_V), eChipIdMaster);
  sendCommandData(WAVESHARE13_REGISTER_TFT_VCOM_POWER, TFT_VCOM_POWER_V,
                  sizeof(TFT_VCOM_POWER_V), eChipIdMaster);
}

void Waveshare13::sendCommandData(uint8_t cmd, const uint8_t *data, int n,
                                  enum eSpectraChipID chipId) {
  if (chipId & eChipIdMaster)
    gpio_set_level(WAVESHARE13_CS_M_PIN, 0);
  if (chipId & eChipIdSlave)
    gpio_set_level(WAVESHARE13_CS_S_PIN, 0);

  m_spi.sendCommand(cmd, WAVESHARE13_DC_PIN);

  if (data && n > 0)
    m_spi.sendData(const_cast<uint8_t *>(data), n, WAVESHARE13_DC_PIN);

  if (chipId & eChipIdMaster)
    gpio_set_level(WAVESHARE13_CS_M_PIN, 1);
  if (chipId & eChipIdSlave)
    gpio_set_level(WAVESHARE13_CS_S_PIN, 1);
}

void Waveshare13::turnOnDisplay(bool leaveOn) {
  sendCommandData(WAVESHARE13_REGISTER_PON, nullptr, 0, eChipIdBoth);
  waitForEpd(60000);

  vTaskDelay(pdMS_TO_TICKS(50));
  sendCommandData(WAVESHARE13_REGISTER_DRF, DRF_V, sizeof(DRF_V), eChipIdBoth);
  waitForEpd(60000);
  vTaskDelay(pdMS_TO_TICKS(50));

  if (!leaveOn) {
    sendCommandData(WAVESHARE13_REGISTER_POF, POF_V, sizeof(POF_V),
                    eChipIdBoth);
    setPanelDeepSleep(true);
  }
}
