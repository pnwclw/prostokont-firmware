/**
 * @file Display.cpp
 * @author Fran Fodor for Soldered
 * @brief Display fuctions for Adafruit_GFX overrides.
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

#include "Display.h"

#include "string.h"

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

Display::Display()
    : Adafruit_GFX(E_INK_WIDTH, E_INK_HEIGHT),
      Graphics(E_INK_WIDTH, E_INK_HEIGHT), image(this) {
  clearDisplay();
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2)
  setRotation(3);
#endif
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
  setRotation(2);
#endif
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE4)
  setRotation(1);
#endif
}

#ifndef COLOR_IMAGE
void Display::preloadScreen() {
  memcpy(m_framebuffer, m_newFramebuffer, m_einkWidth * m_einkHeight / 8);
}
#endif

void Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
  writePixel(x, y, color);
}

uint8_t Display::getRotation() { return rotation; }

esp_err_t Display::showPackedFrame(const uint8_t *frame, size_t len) {
#ifdef COLOR_IMAGE
  const size_t packedFrameBytes = E_INK_WIDTH * E_INK_HEIGHT / 2;
  if (frame == nullptr)
    return ESP_ERR_INVALID_ARG;
  if (len != packedFrameBytes)
    return ESP_ERR_INVALID_SIZE;
  if (m_framebufferColor == nullptr)
    return ESP_ERR_INVALID_STATE;

  memcpy(m_framebufferColor, frame, packedFrameBytes);
  return display();
#else
  (void)frame;
  (void)len;
  return ESP_ERR_NOT_SUPPORTED;
#endif
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void Display::writePixel(int16_t x, int16_t y, uint16_t color) {
  writePixelInternal(x, y, color);
}
