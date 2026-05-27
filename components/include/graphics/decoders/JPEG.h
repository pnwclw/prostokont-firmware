/**
 * @file JPEG.h
 * @author Fran Fodor for Soldered
 * @brief JPEG image decoder.
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

#include "rom/tjpgd.h"
#include "stdlib.h"

#define TJPGD_WORKSPACE_SIZE 16688

struct JpegSrc {
  const uint8_t *data;
  uint32_t index;
  uint32_t size;
};

class Display;

/**
 * @brief Class for decoding JPEG images.
 *
 */
class JPEG {
public:
  /**
   * @brief Construct a new JPEG object.
   *
   * @param inkplate inkplate instance.
   */
  JPEG(Display *inkplate);

  /**
   * @brief Decodes and draws a JPEG image from a raw buffer.
   *
   * @param buf pointer to the JPEG file buffer.
   * @param len length of the buffer in bytes.
   * @param x x coordinate of the top-left corner.
   * @param y y coordinate of the top-left corner.
   * @param dither true to apply dithering.
   * @param invert true to invert pixel values.
   * @return bool true on success, false on memory or decode error.
   */
  bool draw(uint8_t *buf, int32_t len, int x, int y, bool dither = false,
            bool invert = false);

private:
  /**
   * @brief TJpgDec input callback — feeds compressed data to the decoder.
   *
   * @param jdec decoder context.
   * @param buf destination buffer, or null to skip bytes.
   * @param len number of bytes requested.
   * @return UINT number of bytes actually read.
   */
  static UINT inputCallback(JDEC *jdec, BYTE *buf, UINT len);

  /**
   * @brief TJpgDec output callback — draws one decoded MCU block.
   *
   * @param jdec decoder context.
   * @param bitmap pointer to the decoded RGB block.
   * @param rect position and size of the block within the image.
   * @return UINT 1 to continue decoding, 0 to abort.
   */
  static UINT outputCallback(JDEC *jdec, void *bitmap, JRECT *rect);

  Display *m_inkplate;
  int m_x;
  int m_y;
  bool m_invert;
  bool m_dither;
  int64_t m_lastYieldUs;
  uint8_t *m_lineBuf; // RGB line buffer (h * width * 3 bytes, dither path only)
  int m_lineBufH;     // MCU block height, set from first callback
  int m_lineBufY;     // current absolute Y offset in the image

  static JPEG *m_instance;
};