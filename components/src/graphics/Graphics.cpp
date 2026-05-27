/**
 * @file Graphics.cpp
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

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#include "Graphics.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void Graphics::setRotation(uint8_t x) {
  rotation = (x & 3);
  switch (rotation) {
  case 0:
  case 2:
    _width = WIDTH;
    _height = HEIGHT;
    break;
  case 1:
  case 3:
    _width = HEIGHT;
    _height = WIDTH;
    break;
  }
}

uint8_t Graphics::getRotation() { return rotation; }

void Graphics::drawPixel(int16_t x0, int16_t y0, uint16_t color) {
  writePixel(x0, y0, color);
}

void Graphics::drawTextBox(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                           const char *text, uint16_t textSizeMultiplier,
                           const GFXfont *font, uint16_t verticalSpacing,
                           bool showBorder, uint16_t fontSize) {
  int16_t currentX = x0;
  int16_t currentY = y0;

  int16_t textLenght = strlen(text);
  int offset = 0;
  fontSize = (fontSize * 3) / 4; // 1pt = 4/3 px
  int numOfCharactersPerLine = (x1 - x0) / (textSizeMultiplier * fontSize);
  int16_t currentLineLenght = numOfCharactersPerLine;
  this->setTextSize(textSizeMultiplier);
  this->setFont(font);
  if (showBorder) {
    this->drawRect(x0, y0, (x1 - x0), (y1 - y0), 1);
  }
  if (verticalSpacing == 0) {
    verticalSpacing = textSizeMultiplier * fontSize + 6;
  }
  for (int i = y0; i < (y1 - verticalSpacing); i += verticalSpacing) {
    currentY = i;
    this->setCursor(currentX, currentY);

    int remainingLength = textLenght - offset;
    int lineLength = (remainingLength < currentLineLenght) ? remainingLength
                                                           : currentLineLenght;

    // Temporary buffer to hold potential line
    char *buffer = (char *)malloc((lineLength + 1) * sizeof(char));
    memcpy(buffer, text + offset, lineLength);
    buffer[lineLength] = '\0';

    // Find the last space in buffer to wrap at word boundary
    int lastSpaceIndex = -1;
    for (int j = 0; j < lineLength; ++j) {
      if (buffer[j] == ' ')
        lastSpaceIndex = j;
    }

    // If a word gets cut, wrap to the next line
    if ((offset + lineLength < textLenght) &&
        (text[offset + lineLength] != ' ') && (lastSpaceIndex != -1) &&
        ((i + verticalSpacing) < (y1 - verticalSpacing))) {
      lineLength = lastSpaceIndex + 1; // Include the space
    }

    // Allocate space for actual line with null-terminator
    char *textPart = (char *)malloc((currentLineLenght + 1) * sizeof(char));
    memset(textPart, ' ', currentLineLenght);    // Fill with spaces
    memcpy(textPart, text + offset, lineLength); // Copy valid part
    textPart[currentLineLenght] = '\0';

    // Ellipsis on final visible line
    if ((i + verticalSpacing) >= (y1 - verticalSpacing) &&
        (offset + lineLength < textLenght)) {
      textPart[currentLineLenght - 1] = '.';
      textPart[currentLineLenght - 2] = '.';
      textPart[currentLineLenght - 3] = '.';
    }

    this->print(textPart);

    offset += lineLength;
    free(buffer);
    free(textPart);

    if (offset >= textLenght)
      return;
  }
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void Graphics::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                             uint16_t color) {
  static int64_t lastYield = 0;
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j)
      writePixel(x + j, y + i, color);
    // Yield to watchdog at most once per second
    int64_t now = esp_timer_get_time();
    if (now - lastYield > 1000000LL) {
      vTaskDelay(1);
      lastYield = now;
    }
  }
}

void Graphics::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  static int64_t lastYield = 0;
  for (int i = 0; i < h; ++i) {
    writePixel(x, y + i, color);
    // Yield to watchdog at most once per second
    int64_t now = esp_timer_get_time();
    if (now - lastYield > 1000000LL) {
      vTaskDelay(1);
      lastYield = now;
    }
  }
}

void Graphics::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  for (int j = 0; j < w; ++j)
    writePixel(x + j, y, color);
}

void Graphics::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                         uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx >> 1;
  int16_t ystep;

  if (y0 < y1)
    ystep = 1;
  else
    ystep = -1;

  for (; x0 <= x1; x0++) {
    if (steep)
      writePixel(y0, x0, color);
    else
      writePixel(x0, y0, color);
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}