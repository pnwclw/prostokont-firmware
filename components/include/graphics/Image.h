/**
 * @file Image.h
 * @author Fran Fodor for Soldered
 * @brief Drawing black and white/grayscale images.
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

#include "BMP.h"
#include "JPEG.h"
#include "PNG.h"

#ifndef RGB8BIT
#define RGB8BIT(r, g, b)                                                       \
  ((uint8_t)(((uint32_t)(r) * 54 + (uint32_t)(g) * 183 +                       \
              (uint32_t)(b) * 19) >>                                           \
             8))
#endif

class Display;

/**
 * @brief Class for image functions.
 *
 */
class Image {
public:
  /**
   * @brief Construct a new Image object.
   *
   * @param inkplate inkplate instance.
   */
  Image(Display *inkplate);

  /**
   * @brief Calculates dither for given pixel in images.
   *
   * @param px input pixel value (luminance or packed colour).
   * @param i current column index within the row.
   * @param w image width.
   * @param paletted 1 true if the palette entry has already been unpacked to
   * luminance by the caller.
   * @return uint8_t new pixel value.
   *
   * @note called by format decoders via m_inkplate->image when dithering is
   * enabled.
   */
  uint8_t getDitheredPixel(uint32_t px, int i, int w, bool paletted);

  /**
   * @brief Advances the dither row buffers by one row.
   *
   * @param w row width in pixels.
   */
  void ditherSwap(int w);

  /**
   * @brief Draws a image from a raw buffer.
   *
   * @param buf pointer to the image buffer.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to apply Floyd-Steinberg dithering.
   * @param invert true to invert pixel colours.
   * @return bool true on success, false if the format is unrecognised.
   */
  bool draw(uint8_t *buf, int x, int y, bool dither = false,
            bool invert = false);

  /**
   * @brief Draws a JPEG, PNG, or BMP image from a raw buffer.
   *
   * @param buf pointer to the image buffer.
   * @param len length of the buffer in bytes.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to invert pixel colours.
   * @param invert true to apply Floyd-Steinberg dithering.
   * @return bool true on success, false if the format is unrecognised.
   */
  bool draw(uint8_t *buf, int32_t len, int x, int y, bool dither = false,
            bool invert = false);

  /**
   * @brief Draws an image from a URL or an SD card path.
   *
   * @param src URL or file path of the image.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to apply Floyd-Steinberg dithering.
   * @param invert true to invert pixel colours.
   * @return bool true on success, false on download, file, or memory error.
   *
   * @note The source is selected by prefix:
   *       - "https://" → HTTPS download
   *       - "http://"  → HTTP download
   *       - anything else → SD card file (mount point prepended if path is
   * relative)
   */
  bool draw(const char *src, int x, int y, bool dither = false,
            bool invert = false);

  /**
   * @brief Draws a raw 2bpp greyscale bitmap at the given position.
   *
   * @param buf pointer to the 2bpp bitmap buffer.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param w image width in pixels.
   * @param h image height in pixels.
   * @param c color.
   * @return bool true always
   */
  bool draw(const uint8_t *buf, int x, int y, int w, int h, int c);

private:
  /**
   * @brief Allocates and zeroes the dither row buffers.
   *
   * @note Must be called before starting a dithered draw. Allocates two buffers
   *       of BMP_MAX_WIDTH + 2 bytes to allow safe neighbour access at row
   * edges.
   */
  void beginDither();

  /**
   * @brief Frees the dither row buffers after a dithered draw completes.
   *
   */
  void endDither();

  Display *m_inkplate;
  BMP m_bmp;
  JPEG m_jpeg;
  PNG m_png;

  bool m_dither;
  uint8_t *m_ditherBuffer[2];
};