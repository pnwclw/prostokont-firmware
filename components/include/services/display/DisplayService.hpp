/**
 * @file DisplayService.hpp
 * @brief Application-level display service.
 */

#pragma once

#include "Display.h"
#include "esp_err.h"

#include <cstddef>

class DeviceIdentity;

class DisplayService {
public:
  explicit DisplayService(Display &display) : m_display(display) {}

  esp_err_t begin();
  esp_err_t clear();
  esp_err_t showPackedFrame(const uint8_t *frame, size_t len);
  esp_err_t showImage(const uint8_t *image, size_t len, bool dither = true);

  esp_err_t showSetupScreen(const DeviceIdentity &deviceIdentity,
                            const char *apSsid, const char *url,
                            const char *pairingCode,
                            const char *bleName = nullptr);
  esp_err_t showConnecting(const char *target);
  esp_err_t showReady(const char *endpoint);
  esp_err_t showError(const char *message);

private:
  void drawQrCode(const char *payload, int16_t left, int16_t top,
                  int16_t size, uint16_t darkColor, uint16_t lightColor);
  esp_err_t showStatusScreen(const char *title, uint16_t accentColor,
                             const char *line1 = nullptr,
                             const char *line2 = nullptr,
                             const char *line3 = nullptr);

  Display &m_display;
};
