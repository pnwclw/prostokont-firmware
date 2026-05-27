/**
 * @file PNG.cpp
 * @author Fran Fodor for Soldered
 * @brief PNG image decoder.
 *
 * https://github.com/SolderedElectronics/Display-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Display.h"
#include "PNG.h"

PNG *PNG::m_instance = nullptr;
int64_t PNG::m_lastYieldUs = 0;
uint32_t PNG::m_lastDitherY = UINT32_MAX;

PNG::PNG(Display *inkplate)
    : m_inkplate(inkplate), m_x(0), m_y(0), m_invert(false), m_dither(false) {}

bool PNG::draw(uint8_t *buf, int32_t len, int x, int y, bool dither,
               bool invert) {
  m_instance = this;
  m_x = x;
  m_y = y;
  m_dither = dither;
  m_invert = invert;
  m_lastYieldUs = esp_timer_get_time();
  m_lastDitherY = UINT32_MAX;

  ESP_LOGI("PNG", "Starting draw");

  pngle_t *pngle = pngle_new();
  if (!pngle)
    return false;

  pngle_set_draw_callback(pngle, drawCallback);

  int result = pngle_feed(pngle, buf, len);

  pngle_destroy(pngle);

  ESP_LOGI("PNG", "Draw finished");

  return result >= 0;
}

void PNG::drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w,
                       uint32_t h, const uint8_t rgba[4]) {
  if (!m_instance)
    return;

  // yield to IDLE task periodically so the task watchdog doesn't trigger
  int64_t now = esp_timer_get_time();
  if (now - m_instance->m_lastYieldUs >= 1000000LL) {
    vTaskDelay(1);
    m_instance->m_lastYieldUs = esp_timer_get_time();
  }

  // skip fully transparent pixels
  if (!rgba[3])
    return;

  uint8_t r = rgba[0];
  uint8_t g = rgba[1];
  uint8_t b = rgba[2];

  // 1-bit depth: expand to full black or white
  pngle_ihdr_t *ihdr = pngle_get_ihdr(pngle);
  if (ihdr->depth == 1)
    r = g = b = (b ? 0xFF : 0);

  if (m_instance->m_dither) {
    for (uint32_t j = 0; j < h; ++j) {
      uint32_t rowY = y + j;

      if (rowY != m_lastDitherY && m_lastDitherY != UINT32_MAX)
        m_instance->m_inkplate->image.ditherSwap(ihdr->width);
      m_lastDitherY = rowY;

      for (uint32_t i = 0; i < w; ++i) {
        uint8_t val;
#ifdef COLOR_IMAGE
        val = m_instance->m_inkplate->image.getDitheredPixel(r, g, b, x + i,
                                                             ihdr->width);
        if (m_instance->m_invert && val < 2)
          val ^= 1;
#else
        val = m_instance->m_inkplate->image.getDitheredPixel(
            RGB8BIT(r, g, b), x + i, ihdr->width, false);
        if (m_instance->m_invert)
          val ^= 7;
        if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
          val = (~val >> 2) & 1;
#endif
        m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i,
                                          m_instance->m_y + rowY, val);
      }
    }
  } else {
    uint8_t val;
#ifdef COLOR_IMAGE
    val = m_instance->m_inkplate->image.findClosestPalette(r, g, b);
    if (m_instance->m_invert && val < 2)
      val ^= 1;
#else
    val = RGB3BIT(r, g, b);
    if (m_instance->m_invert)
      val ^= 7;
    if (m_instance->m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
      val = (~val >> 2) & 1;
#endif
    for (uint32_t j = 0; j < h; ++j)
      for (uint32_t i = 0; i < w; ++i)
        m_instance->m_inkplate->drawPixel(m_instance->m_x + x + i,
                                          m_instance->m_y + y + j, val);
  }
}
