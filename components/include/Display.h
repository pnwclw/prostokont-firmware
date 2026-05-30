/**
 * @file Display.h
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

#pragma once

#include "graphics/Graphics.h"
#include "panels/PanelFactory.hpp"
#include "services/network/HttpClient.hpp"

#include <cstddef>

#if PROSTOKONT_BOARD_COLOR_IMAGE
#include "ImageColor.h"
#else
#include "Image.h"
#endif

/**
 * @brief Class for display overrides.
 *
 */
class Display : public Graphics, public prostokont::PanelFactory::PanelType {
public:
  /**
   * @brief Construct a new Display object.
   *
   */
  Display();

#if !PROSTOKONT_BOARD_COLOR_IMAGE
  /**
   * @brief Copies the framebuffer to partial for deepsleep restore.
   *
   */
  void preloadScreen();
#endif
  /**
   * @brief Draw a single pixel — Adafruit_GFX override.
   *
   * @param x x coordinate.
   * @param y y coordinate.
   * @param color pixel value (0-7 in grayscale mode, 0-1 in B&W mode).
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /**
   * @brief Return the current display rotation (0-3, matching Adafruit_GFX
   * convention).
   *
   * @return uint8_t rotation index.
   */
  uint8_t getRotation() override;

  /**
   * @brief Copy a packed color framebuffer into the active display buffer and
   * flush it to the panel.
   *
   * @param frame packed framebuffer data.
   * @param len framebuffer size in bytes.
   * @return esp_err_t ESP_OK on success, or an error code if unsupported or
   * invalid.
   */
  esp_err_t showPackedFrame(const uint8_t *frame, size_t len);

#if PROSTOKONT_BOARD_COLOR_IMAGE
  ImageColor image;
#else
  Image image;
#endif

  HttpClient httpClient;

private:
  /**
   * @brief Write a single pixel to the frame buffer.
   *
   * @param x x coordinate.
   * @param y y coordinate.
   * @param color pixel value.
   */
  void writePixel(int16_t x, int16_t y, uint16_t color);
};
