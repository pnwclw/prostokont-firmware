/**
 * @file PNG.h
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

#pragma once

#include "stdlib.h"

#include "pngle.h"

class Display;

/**
 * @brief Class for decoding PNG images.
 *
 */
class PNG {
public:
  /**
   * @brief Construct a new PNG object.
   *
   * @param inkplate inkplate instance.
   */
  PNG(Display *inkplate);

  /**
   * @brief
   *
   * @param buf pointer to the PNG file buffer.
   * @param len length of the buffer in bytes.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to apply dithering.
   * @param invert true to invert pixel values.
   * @return bool true on success, false on decode or memory error.
   */
  bool draw(uint8_t *buf, int32_t len, int x, int y, bool dither = false,
            bool invert = false);

private:
  /**
   * @brief pngle pixel callback — draws one decoded pixel block.
   *
   * @param  pngle pngle decoder context
   * @param  x x position of the block within the image
   * @param  y y position of the block within the image
   * @param  w block width in pixels
   * @param  h block height in pixels
   * @param  rgba decoded pixel color (R, G, B, A)
   */
  static void drawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w,
                           uint32_t h, const uint8_t rgba[4]);

  Display *m_inkplate;
  int m_x;
  int m_y;
  bool m_invert;
  bool m_dither;

  static PNG *m_instance;
  static int64_t m_lastYieldUs;
  static uint32_t m_lastDitherY;
};