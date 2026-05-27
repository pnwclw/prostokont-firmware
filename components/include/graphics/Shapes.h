/**
 * @file Shapes.h
 * @author Fran Fodor for Soldered
 * @brief Helper for drawing graphics.
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

#include "sdkconfig.h"
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6) ||                                \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
#include "panels/inkplate/Inkplate6.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
#include "panels/inkplate/Inkplate6Color.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE10)
#include "panels/inkplate/Inkplate10.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
#include "panels/inkplate/Inkplate13.h"
#elif defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
#include "panels/waveshare/Waveshare13.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE5)
#include "panels/inkplate/Inkplate5.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE4)
#include "panels/inkplate/Inkplate4.h"
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE2)
#include "panels/inkplate/Inkplate2.h"
#else
#error                                                                         \
    "No Display board selected. Choose a board in menuconfig -> Display Board."
#endif

#include "Adafruit_GFX.h"

#define maxVer 100
#define maxHt E_INK_HEIGHT

/**
 * @brief Class for drawing shapes
 *
 */
class Shapes : virtual public Adafruit_GFX {
public:
  /**
   * @brief Construct a new Shapes object.
   *
   * @param w width.
   * @param h height.
   */
  Shapes(int16_t w, int16_t h) : Adafruit_GFX(w, h) {};

  /**
   * @brief Draw a pixel at coordinates.
   *
   * @param x x coordinate.
   * @param y y coordinate.
   * @param color pixel color.
   */
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

  /**
   * @brief Draw empty elipse shape.
   *
   * @param rx x plane radius.
   * @param ry y plane radius.
   * @param xc x plane central point.
   * @param yc y plane central point.
   * @param c color.
   */
  void drawElipse(int rx, int ry, int xc, int yc, int c);

  /**
   * @brief Draw filled elipse shape.
   *
   * @param rx x plane radius.
   * @param ry y plane radius.
   * @param xc x plane central point.
   * @param yc y plane central point.
   * @param c color.
   */
  void fillElipse(int rx, int ry, int xc, int yc, int c);

  /**
   * @brief Draw thick filled line.
   *
   * @param x1 x plane starting point.
   * @param y1 y plane starting point.
   * @param x2 x plane end point.
   * @param y2 y plane end point.
   * @param color line color.
   * @param thickness line thickness in pixels.
   */
  void drawThickLine(int x1, int y1, int x2, int y2, int color,
                     float thickness);

  /**
   * @brief Draw thick gradient line.
   *
   * @param x1 x plane starting point.
   * @param y1 y plane starting point.
   * @param x2 x plane end point.
   * @param y2 y plane end point.
   * @param color1 starting color for gradient line.
   * @param color2 ending color for gradient line.
   * @param thickness line thickness in pixels
   *
   * @note Color 1 should always be less than color 2.
   */
  void drawGradientLine(int x1, int y1, int x2, int y2, int color1, int color2,
                        float thickness = -1);

  /**
   * @brief Draw text with prev. defined size with shadow.
   *
   * @param x text cursor for the x position.
   * @param y text cursor for the y position.
   * @param _text string that needs to be printed.
   * @param _colorText color of the text.
   * @param _colorShadow color of the shadow "below" the text.
   */
  void drawTextWithShadow(int x, int y, const char *_text, uint8_t _colorText,
                          uint8_t _colorShadow);

  /**
   * @brief Draw polygon line by line (horizontally).
   *
   * @param x pointer to x plane point.
   * @param y pointer to y plane point.
   * @param n number of iterations.
   * @param color polygon color.
   */
  void drawPolygon(int *x, int *y, int n, int color);

  /**
   * @brief Draw filled polygon line by line (horizontally).
   *
   * @param x pointer to x plane point.
   * @param y pointer to y plane point.
   * @param n number of iterations.
   * @param color polygon color.
   */
  void fillPolygon(int *x, int *y, int n, int color);

private:
  struct EdgeBucket {
    int ymax;
    float xofymin;
    float slopeinverse;
  };

  struct edgeTableTuple {
    int countEdgeBucket;
    EdgeBucket buckets[maxVer];
  };

  /**
   * @brief Initiate edge table and sets all values inside struct to 0.
   *
   */
  void initedgeTable();

  /**
   * @brief Sort buckets inside edgeTableTuple.
   *
   * @param ett pointer to edgeTableTuple to be sorted.
   */
  void insertionSort(edgeTableTuple *ett);

  /**
   * @brief Store values in tuple structure.
   *
   * @param receiver pointer to edgeTableTuple structure.
   * @param ym edgeTableTuple->ymax value.
   * @param xm edgeTableTuple->xofymin value.
   * @param slopInv edgeTableTuple->slopeInverse value.
   */
  void storeEdgeInTuple(edgeTableTuple *receiver, int ym, int xm,
                        float slopInv);

  /**
   * @brief Calculate edge values of edgeTableTuple and stores them.
   *
   * @param x1 x plane starting position
   * @param y1 y plane starting position
   * @param x2 x plane ending position
   * @param y2 y plane ending position
   */
  void storeEdgeInTable(int x1, int y1, int x2, int y2);

  /**
   * @brief Remove edge by given yy.
   *
   * @param tup pointer to edgeTableTuple to work on.
   * @param yy value to remove from edgeTableTuple.
   */
  void removeEdgeByYmax(edgeTableTuple *tup, int yy);

  /**
   * @brief Update all xofymin by adding slopeinverse value.
   *
   * @param tup pointer to edgeTableTuple to work on
   */
  void updatexbyslopeinv(edgeTableTuple *tup);

  /**
   * @brief Draw horizontal line based on edge table.
   *
   * @param c color
   */
  void scanlineFill(uint8_t c);

  // virtual void startWrite(void) = 0;
  virtual void writePixel(int16_t x, int16_t y, uint16_t color) = 0;
  virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                             uint16_t color) = 0;
  virtual void writeFastVLine(int16_t x, int16_t y, int16_t h,
                              uint16_t color) = 0;
  virtual void writeFastHLine(int16_t x, int16_t y, int16_t w,
                              uint16_t color) = 0;
  virtual void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         uint16_t color) = 0;
  // virtual void endWrite(void) = 0;

  edgeTableTuple *edgeTable, activeEdgeTuple;
};
