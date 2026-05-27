/**
 * @file ImageColor.h
 * @author Fran Fodor for Soldered
 * @brief Drawing color images.
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

#include "DitherKernels.h"

static constexpr uint8_t DITHER_ROW_COUNT =
    4; // ring buffer; covers all kernels (max height 3)
static constexpr uint8_t DITHER_ROW_MASK = DITHER_ROW_COUNT - 1;

#define RED8(a) (((a) >> 16) & 0xff)
#define GREEN8(a) (((a) >> 8) & 0xff)
#define BLUE8(a) (((a)) & 0xff)

/**
 * @brief Dither kernel.
 *
 */
typedef enum {
  FloydSteinberg = 0,
  JarvisJudiceNinke,
  Atkinson,
  Burkes,
  Stucki,
  SierraLite,
  ReducedDiffusion // Floyd-Steinberg pattern at ~69% strength — more vibrant,
                   // less washed-out
} DitherKernel;

class Display;

/**
 * @brief Class for image functions.
 *
 */
class ImageColor {
public:
  /**
   * @brief Construct a new Image Color object.
   *
   * @param inkplate inkplate instance
   */
  ImageColor(Display *inkplate);

  /**
   * @brief Finds the closest palette entry to the given RGB color.
   *
   * @param r red channel value (0–255).
   * @param g green channel value (0–255).
   * @param b blue channel value (0–255).
   * @return uint8_t panel color code of the closest matching color.
   */
  uint8_t findClosestPalette(int16_t r, int16_t g, int16_t b);

  /**
   * @brief Returns the dithered palette index for one pixel.
   *
   * @param r red channel value (0–255).
   * @param g green channel value (0–255).
   * @param b blue channel value (0–255).
   * @param i column index within the current row.
   * @param w row width in pixels.
   * @return uint8_t panel color code of the output color for this pixel.

   */
  uint8_t getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i, int w);

  /**
   * @brief Advances the dither error buffer to the next row.
   *
   * @param w unused.
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

  /**
   * @brief Sets the active dithering kernel.
   *
   * @param kernel kernel to use for subsequent dithered draw calls.
   */
  void setDitherKernel(DitherKernel kernel);

  // called by format decoders when dithering is enabled
  uint8_t *m_palette;
  uint32_t pallete[7];  // board color palette, packed 0x00RRGGBB
  uint8_t palletteSize; // number of valid entries in pallete[]

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

  uint8_t findClosestPaletteEntry(int16_t r, int16_t g, int16_t b);

  Display *m_inkplate;
  BMP m_bmp;
  JPEG m_jpeg;
  PNG m_png;
  const DitherKernelDef *m_currentKernel = &DITHER_KERNELS[0];
  uint8_t m_paletteColorCode[7]; // panel color code for each pallete[] entry

  int16_t *m_ditherR[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators
                                        // — red channel
  int16_t *m_ditherG[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators
                                        // — green channel
  int16_t *m_ditherB[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators
                                        // — blue channel
  int m_rowIdx; // current row position in the ring buffer
};
