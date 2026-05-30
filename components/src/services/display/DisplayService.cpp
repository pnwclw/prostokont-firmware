/**
 * @file DisplayService.cpp
 * @brief Application-level display service.
 */

#include "services/display/DisplayService.hpp"

#include "config/AppConfig.hpp"
#include "graphics/ImageColor.h"
#include "graphics/qrcodegen/qrcodegen.h"
#include "services/board/BoardProfile.hpp"
#include "services/device/DeviceIdentity.hpp"

namespace {

constexpr uint16_t kColorBlack = 0;
constexpr uint16_t kColorWhite = 1;
constexpr int16_t kQrSize = 260;
constexpr int16_t kQrLeft = E_INK_WIDTH - kQrSize - 80;
constexpr int16_t kQrTop = 120;

} // namespace

esp_err_t DisplayService::begin() {
  m_display.setRotation(currentBoardProfile().defaultRotation);
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

  const BoardProfile &board = currentBoardProfile();
  m_display.clearDisplay();

#if PROSTOKONT_BOARD_COLOR_IMAGE
  if (board.preferReducedDither) {
    if (dither)
      m_display.image.setDitherKernel(ReducedDiffusion);
    else
      m_display.image.setDitherKernel(FloydSteinberg);
  }
#endif

  bool drawn = m_display.image.draw(const_cast<uint8_t *>(image),
                                    static_cast<int32_t>(len), 0, 0, dither,
                                    false);

#if PROSTOKONT_BOARD_COLOR_IMAGE
  if (board.preferReducedDither)
    m_display.image.setDitherKernel(FloydSteinberg);
#endif

  if (!drawn) {
    return ESP_ERR_NOT_SUPPORTED;
  }

  return m_display.display();
}

esp_err_t DisplayService::showSetupScreen(const DeviceIdentity &deviceIdentity,
                                          const char *apSsid, const char *url,
                                          const char *pairingCode,
                                          const char *bleName) {
  const BoardProfile &board = currentBoardProfile();
  m_display.clearDisplay();
  m_display.setTextColor(board.accentColor);
  m_display.setTextSize(4);
  m_display.setCursor(40, 70);
  m_display.println("Setup");

  m_display.drawLine(40, 90, kQrLeft - 30, 90, board.accentColor);
  m_display.setTextColor(kColorBlack);
  m_display.setTextSize(2);

  int16_t y = 140;
  m_display.setCursor(40, y);
  m_display.println(deviceIdentity.name().c_str());
  y += 45;

  if (apSsid != nullptr && apSsid[0] != '\0') {
    m_display.setCursor(40, y);
    m_display.print("WiFi: ");
    m_display.println(apSsid);
    y += 45;
  }

  if (bleName != nullptr && bleName[0] != '\0') {
    m_display.setCursor(40, y);
    m_display.print("BLE: ");
    m_display.println(bleName);
    y += 45;
  }

  if (url != nullptr && url[0] != '\0') {
    m_display.setCursor(40, y);
    m_display.println("Open setup URL:");
    y += 35;
    m_display.setCursor(40, y);
    m_display.println(url);
    y += 45;
  }

  if (pairingCode != nullptr && pairingCode[0] != '\0') {
    m_display.setCursor(40, y);
    m_display.print("Code: ");
    m_display.println(pairingCode);
  }

  drawQrCode(url, kQrLeft, kQrTop, kQrSize, kColorBlack, kColorWhite);
  return m_display.display();
}

esp_err_t DisplayService::showConnecting(const char *target) {
  return showStatusScreen("Connecting", currentBoardProfile().accentColor, target);
}

esp_err_t DisplayService::showReady(const char *endpoint) {
  const BoardProfile &board = currentBoardProfile();
  m_display.clearDisplay();
  m_display.setTextColor(board.accentColor);
  m_display.setTextSize(4);
  m_display.setCursor(40, 70);
  m_display.println("Ready");

  m_display.drawLine(40, 90, kQrLeft - 30, 90, board.accentColor);
  m_display.setTextColor(kColorBlack);
  m_display.setTextSize(2);
  m_display.setCursor(40, 150);
  m_display.println(config::kProductName);
  m_display.setCursor(40, 200);
  m_display.println("Local endpoint:");
  m_display.setCursor(40, 240);
  m_display.println(endpoint);

  drawQrCode(endpoint, kQrLeft, kQrTop, kQrSize, kColorBlack, kColorWhite);
  return m_display.display();
}

esp_err_t DisplayService::showError(const char *message) {
  return showStatusScreen("Error", currentBoardProfile().errorColor, message);
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

void DisplayService::drawQrCode(const char *payload, int16_t left, int16_t top,
                                int16_t size, uint16_t darkColor,
                                uint16_t lightColor) {
  if (payload == nullptr || payload[0] == '\0')
    return;

  static uint8_t qrTemp[qrcodegen_BUFFER_LEN_MAX];
  static uint8_t qrCode[qrcodegen_BUFFER_LEN_MAX];
  if (!qrcodegen_encodeText(payload, qrTemp, qrCode, qrcodegen_Ecc_MEDIUM, 1,
                            10, qrcodegen_Mask_AUTO, true)) {
    return;
  }

  const int qrSize = qrcodegen_getSize(qrCode);
  if (qrSize <= 0)
    return;

  const int border = 2;
  const int fullSize = qrSize + border * 2;
  const int scale = size / fullSize;
  if (scale <= 0)
    return;

  const int renderedSize = fullSize * scale;
  const int16_t originX = left + (size - renderedSize) / 2;
  const int16_t originY = top + (size - renderedSize) / 2;

  m_display.drawRect(left - 8, top - 8, size + 16, size + 16, darkColor);
  m_display.fillRect(left, top, size, size, lightColor);

  for (int y = 0; y < fullSize; ++y) {
    for (int x = 0; x < fullSize; ++x) {
      const bool dark =
          x >= border && y >= border && x < qrSize + border &&
          y < qrSize + border &&
          qrcodegen_getModule(qrCode, x - border, y - border);
      if (!dark)
        continue;

      m_display.fillRect(originX + x * scale, originY + y * scale, scale,
                         scale, darkColor);
    }
  }
}
