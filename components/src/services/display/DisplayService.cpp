/**
 * @file DisplayService.cpp
 * @brief Application-level display service.
 */

#include "services/display/DisplayService.hpp"

#include "config/AppConfig.hpp"
#include "graphics/ImageColor.h"
#include "services/device/DeviceIdentity.hpp"

namespace {

constexpr uint16_t kColorBlack = 0;
constexpr uint16_t kColorWhite = 1;

#if defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
constexpr uint16_t kColorAccent = WAVESHARE13_BLUE;
constexpr uint16_t kColorError = WAVESHARE13_RED;
#else
constexpr uint16_t kColorAccent = kColorBlack;
constexpr uint16_t kColorError = kColorBlack;
#endif

} // namespace

esp_err_t DisplayService::begin() {
  m_display.setRotation(0);
  return ESP_OK;
}

esp_err_t DisplayService::clear() {
  m_display.clearDisplay();
  return m_display.display();
}

esp_err_t DisplayService::showPackedFrame(const uint8_t *frame, size_t len) {
  return m_display.showPackedFrame(frame, len);
}

esp_err_t DisplayService::showImage(const uint8_t *image, size_t len,
                                    bool dither) {
  if (image == nullptr || len == 0)
    return ESP_ERR_INVALID_ARG;

  m_display.clearDisplay();

#if defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  if (dither)
    m_display.image.setDitherKernel(ReducedDiffusion);
  else
    m_display.image.setDitherKernel(FloydSteinberg);
#endif

  bool drawn = m_display.image.draw(const_cast<uint8_t *>(image),
                                    static_cast<int32_t>(len), 0, 0, dither,
                                    false);

#if defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  m_display.image.setDitherKernel(FloydSteinberg);
#endif

  if (!drawn) {
    return ESP_ERR_NOT_SUPPORTED;
  }

  return m_display.display();
}

esp_err_t DisplayService::showStartupColorBars() {
#if defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  constexpr uint16_t kStripeColors[] = {WAVESHARE13_BLACK, WAVESHARE13_WHITE,
                                        WAVESHARE13_YELLOW, WAVESHARE13_RED,
                                        WAVESHARE13_BLUE, WAVESHARE13_GREEN};
#else
  constexpr uint16_t kStripeColors[] = {kColorBlack, kColorWhite, kColorAccent,
                                        kColorError};
#endif

  constexpr size_t kStripeCount = sizeof(kStripeColors) / sizeof(kStripeColors[0]);
  const int16_t stripeWidth = E_INK_WIDTH / static_cast<int16_t>(kStripeCount);

  m_display.clearDisplay();
  for (size_t i = 0; i < kStripeCount; ++i) {
    const int16_t x = static_cast<int16_t>(i) * stripeWidth;
    const int16_t width =
        (i + 1 == kStripeCount) ? (E_INK_WIDTH - x) : stripeWidth;
    m_display.fillRect(x, 0, width, E_INK_HEIGHT, kStripeColors[i]);
  }

  return m_display.display();
}

esp_err_t DisplayService::showSetupScreen(const DeviceIdentity &deviceIdentity,
                                          const char *apSsid, const char *url,
                                          const char *pairingCode) {
  return showStatusScreen("Setup", kColorAccent, deviceIdentity.name().c_str(),
                          apSsid, pairingCode != nullptr ? pairingCode : url);
}

esp_err_t DisplayService::showConnecting(const char *target) {
  return showStatusScreen("Connecting", kColorAccent, target);
}

esp_err_t DisplayService::showReady(const char *endpoint) {
  return showStatusScreen("Ready", kColorAccent, config::kProductName, endpoint);
}

esp_err_t DisplayService::showError(const char *message) {
  return showStatusScreen("Error", kColorError, message);
}

esp_err_t DisplayService::showStatusScreen(const char *title,
                                           uint16_t accentColor,
                                           const char *line1,
                                           const char *line2,
                                           const char *line3) {
  m_display.clearDisplay();
  m_display.setTextColor(accentColor);
  m_display.setTextSize(4);
  m_display.setCursor(40, 70);
  m_display.println(title);

  m_display.setTextColor(kColorBlack);
  m_display.drawLine(40, 90, 420, 90, accentColor);
  m_display.setTextSize(2);

  int16_t y = 150;
  const char *lines[] = {line1, line2, line3};
  for (const char *line : lines) {
    if (line == nullptr || line[0] == '\0')
      continue;

    m_display.setCursor(40, y);
    m_display.println(line);
    y += 50;
  }

  m_display.setTextColor(kColorWhite);
  return m_display.display();
}
