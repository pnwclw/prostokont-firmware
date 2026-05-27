/**
 * @file Graphics.h
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

#include "GraphicsDefs.h"
#include "Shapes.h"

/**
 * @brief Class for graphics helpers.
 *
 */
class Graphics : public Shapes {
public:
  /**
   * @brief Construct a new Graphics object.
   *
   * @param w width.
   * @param h height.
   */
  Graphics(int16_t w, int16_t h) : Adafruit_GFX(w, h), Shapes(w, h) {};

  /**
   * @brief Set the width and height modified by current rotation.
   *
   * @param r screen rotation, 0 is normal, 1 is left, 2 is upsidedown and 3 is
   * right.
   */
  void setRotation(uint8_t r);

  /**
   * @brief Get the screen rotation.
   *
   * @return uint8_t 0 is normal, 1 is left, 2 is upsidedown and 3 is right.
   */
  uint8_t getRotation();

  /**
   * @brief Calls drawPixes for different screen sizes.
   *
   * @param x x position, will change depending on rotation.
   * @param y y position, will change depending on rotation.
   * @param color pixel color, in 3bit mode have values in range 0-7.
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;

  /**
   * @brief Draws a text box with optional border and word-wrapped content.
   *
   * @param x0 top-left x-coordinate of the text box.
   * @param y0 top-left y-coordinate of the text box.
   * @param x1 bottom-right x-coordinate of the text box.
   * @param x2 bottom-right y-coordinate of the text box.
   * @param text null-terminated string to be rendered inside the box.
   * @param textSize size multiplier for the text.
   * @param font pointer to the font to use for rendering the text.
   * @param vericalSpacing vertical spacing (in pixels) between lines of text;
   * if 0 or NULL, defaults to text height + padding.
   * @param showBorder whether to draw a border around the text box.
   * @param fontSize font size in points (pt)
   *
   * @note This function renders a block of text inside the defined rectangular
   * area. It automatically wraps words to the next line if they exceed the
   * available width. If the text does not fit entirely, an ellipsis ("...") is
   * added to the final visible line. Text is padded with spaces to keep a
   * consistent line length.
   */
  void drawTextBox(int16_t x0, int16_t y0, int16_t x1, int16_t x2,
                   const char *text, uint16_t textSize = 1,
                   const GFXfont *font = NULL, uint16_t vericalSpacing = 0,
                   bool showBorder = false, uint16_t fontSize = 8);

private:
  /**
   * @brief Write a pixel at position.
   *
   * @param x x-coodrinate.
   * @param y y-coordinate.
   * @param color pixel color.
   */
  void writePixel(int16_t x, int16_t y, uint16_t color) override = 0;

  /**
   * @brief Write filled rectangle starting at x,y position.
   *
   * @param x upper left corner x position for rectangle.
   * @param y upper right corner y position for rectangle.
   * @param w rectangle width.
   * @param h rectangle height.
   * @param color pixel color, in 3bit mode have values in range 0-7.
   */
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color) override;

  /**
   * @brief Write vertical line starting at x,y position.
   *
   * @param x starting x position for vertical line.
   * @param y starting y position for vertical line
   * @param h vertical line height.
   * @param color pixel color, in 3bit mode have values in range 0-7.
   */
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;

  /**
   * @brief Write horizontal line starting at x,y position.
   *
   * @param x starting x position for horizontal line.
   * @param y starting y position for horizontal line.
   * @param w horizontal line width.
   * @param color pixel color, in 3bit mode have values in range 0-7.
   */
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;

  /**
   * @brief Write line at the degree.
   *
   * @param x0 starting x position for line.
   * @param y0 starting y position for line.
   * @param x1 ending x position for line.
   * @param y1 ending y position for line.
   * @param color pixel color, in 3bit mode have values in range 0-7.
   */
  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                 uint16_t color) override;
};