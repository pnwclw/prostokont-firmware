/**
 * @file BMP.h
 * @author Fran Fodor for Soldered
 * @brief BMP image decoder.
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

#include <stdlib.h>

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE13) ||                              \
    defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
#define BMP_MAX_WIDTH 1600
#else
#define BMP_MAX_WIDTH 800
#endif
#define BMP_MAX_ROW_SIZE (BMP_MAX_WIDTH * 4) // 32bpp max
#define BMP_ROWSIZE(w, c) ((((w) * (c) + 31) / 32) * 4)

#ifndef RGB3BIT
#define RGB3BIT(r, g, b)                                                       \
  ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 +                       \
              (uint32_t)(b) * 19) >>                                           \
             13))
#endif
#ifndef RGB8BIT
#define RGB8BIT(r, g, b)                                                       \
  ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 +                       \
              (uint32_t)(b) * 19) >>                                           \
             8))
#endif

/**
 * @brief Parsed fields from a BMP file header.
 *
 */
struct BitmapHeader {
  uint32_t startRAW;
  int32_t width;
  int32_t height;
  uint16_t color;
};

class Display;

/**
 * @brief Class for decoding BMP images.
 *
 */
class BMP {
public:
  /**
   * @brief Construct a new BMP object.
   *
   * @param inkplate inkplate instance.
   */
  BMP(Display *inkplate);

  /**
   * @brief Decodes and draws a BMP image from a raw buffer.
   *
   * @param buf pointer to the BMP file buffer.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to apply dithering.
   * @param invert true to invert pixel values.
   * @return bool true on success, false if the header is invalid.
   */
  bool draw(uint8_t *buf, int x, int y, bool dither = false,
            bool invert = false);

private:
  /**
   * @brief Parses the BMP file header and loads the color table if present.
   *
   * @param buf pointer to the BMP file buffer.
   * @param header output struct to populate.
   */
  void readHeader(uint8_t *buf, BitmapHeader *header);

  /**
   * @brief Validates that the bit depth is supported.
   *
   * @param header parsed header to check.
   * @return bool true if the bit depth is 1, 4, 8, 16, 24, or 32.
   */
  bool isValid(BitmapHeader *header);

  /**
   * @brief Decodes and draws one row of pixels.
   *
   * @param x x coordinate of the left edge of the row.
   * @param y row index from the top of the image.
   * @param header parsed BMP header.
   * @param dither true to invert pixel values.
   * @param invert true to apply dithering.
   */
  void drawLine(int16_t x, int16_t y, BitmapHeader *header, bool dither,
                bool invert);

  Display *m_inkplate;
  uint8_t m_pixelBuffer[BMP_MAX_ROW_SIZE];
  uint32_t m_paletteRGB[256]; // packed 0x00RRGGBB per indexed-BMP palette entry
};
