/**
 * @file   Image.cpp
 * @author Fran Fodor for Soldered
 * @brief  Drawing black and white/grayscale images.
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

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "string.h"

#include "Image.h"
#include "Display.h"

static const char *TAG = "Image";

/* -------------------------------------------------------------------------- */
/*                              Public functions */
/* -------------------------------------------------------------------------- */

Image::Image(Display *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate) {
  m_dither = false;
  m_ditherBuffer[0] = m_ditherBuffer[1] = nullptr;
}

uint8_t Image::getDitheredPixel(uint32_t px, int i, int w, bool paletted) {
  /* paletted: shared decoder already unpacked palette entry to luminance */

  if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE)
    px = (uint16_t)px >> 1;

  uint16_t sum = (uint16_t)m_ditherBuffer[0][i] + (uint16_t)px;
  uint8_t oldPixel = sum > 0xFF ? 0xFF : (uint8_t)sum;

  uint8_t newPixel =
      oldPixel &
      (m_inkplate->getDisplayMode() == BLACK_AND_WHITE ? 0x80 : 0xE0);
  uint8_t quantError = oldPixel - newPixel;

  /* distribute quantisation error: right=7/16, below-left=3/16, below=5/16,
   * below-right=1/16 */
  m_ditherBuffer[1][i] += (quantError * 5) >> 4;
  if (i < w - 1) { // explicit upper bound (not just != w-1)
    m_ditherBuffer[0][i + 1] += (quantError * 7) >> 4;
    m_ditherBuffer[1][i + 1] += (quantError * 1) >> 4;
  }
  if (i > 0) // explicit lower bound
    m_ditherBuffer[1][i - 1] += (quantError * 3) >> 4;

  return newPixel >> 5;
}

void Image::ditherSwap(int w) {
  for (int i = 0; i < w; ++i) {
    m_ditherBuffer[0][i] = m_ditherBuffer[1][i];
    m_ditherBuffer[1][i] = 0;
  }
}

bool Image::draw(uint8_t *buf, int x, int y, bool dither, bool invert) {
  m_dither = dither;
  if (dither)
    beginDither();

  bool result = false;
  if (buf[0] == 0x42 && buf[1] == 0x4D) /* BMP magic bytes */
    result = m_bmp.draw(buf, x, y, dither, invert);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither)
    endDither();
  m_dither = false;
  return result;
}

bool Image::draw(uint8_t *buf, int32_t len, int x, int y, bool dither,
                 bool invert) {
  m_dither = dither;
  if (dither)
    beginDither();

  bool result = false;
  if (buf[0] == 0xFF && buf[1] == 0xD8) /* JPEG magic bytes */
    result = m_jpeg.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E &&
           buf[3] == 0x47) /* PNG magic bytes */
    result = m_png.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x42 && buf[1] == 0x4D) /* BMP magic bytes */
    result = m_bmp.draw(buf, x, y, dither, invert);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither)
    endDither();
  m_dither = false;
  return result;
}

bool Image::draw(const char *src, int x, int y, bool dither, bool invert) {
  int32_t len = 0;
  uint8_t *buf = nullptr;

  if (strncmp(src, "https://", 8) == 0) {
    len = 8 * 1024 * 1024;
    if (m_inkplate->httpClient.downloadSecure(src, &buf, &len) != ESP_OK ||
        buf == nullptr) {
      ESP_LOGE(TAG, "Failed to download: %s", src);
      return false;
    }
  } else if (strncmp(src, "http://", 7) == 0) {
    len = 8 * 1024 * 1024;
    if (m_inkplate->httpClient.download(src, &buf, &len) != ESP_OK ||
        buf == nullptr) {
      ESP_LOGE(TAG, "Failed to download: %s", src);
      return false;
    }
  } else {
    char fullPath[256];
    if (src[0] == '/')
      snprintf(fullPath, sizeof(fullPath), "%s", src);
    else
      snprintf(fullPath, sizeof(fullPath), "%s/%s", m_inkplate->getMountPoint(),
               src);

    FILE *f = fopen(fullPath, "rb");
    if (!f) {
      ESP_LOGE(TAG, "Failed to open: %s", fullPath);
      return false;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = (uint8_t *)malloc(len);
    if (!buf) {
      fclose(f);
      ESP_LOGE(TAG, "Out of memory (%ld bytes)", len);
      return false;
    }

    fread(buf, 1, len, f);
    fclose(f);
  }

  bool result = draw(buf, len, x, y, dither, invert);
  free(buf);
  return result;
}

bool Image::draw(const uint8_t *buf, int x, int y, int w, int h, int c) {
  int64_t lastYieldUs = esp_timer_get_time();

  if (m_inkplate->getDisplayMode() == BLACK_AND_WHITE) {
    m_inkplate->drawBitmap(x, y, buf, w, h, c);
  } else {
    uint8_t rem = w & 1;
    int xSize = (w >> 1) + rem;
    int i, j;

    for (i = 0; i < h; i++) {
      int64_t now = esp_timer_get_time();
      if (now - lastYieldUs >= 1000000LL) {
        vTaskDelay(1);
        lastYieldUs = esp_timer_get_time();
      }
      for (j = 0; j < xSize - 1; j++) {
        m_inkplate->drawPixel((j * 2) + x, i + y,
                              (*(buf + xSize * i + j) >> 4) >> 1);
        m_inkplate->drawPixel((j * 2) + 1 + x, i + y,
                              (*(buf + xSize * i + j) & 0x0f) >> 1);
      }
      m_inkplate->drawPixel((j * 2) + x, i + y,
                            (*(buf + xSize * i + j) >> 4) >> 1);
      if (rem == 0)
        m_inkplate->drawPixel((j * 2) + 1 + x, i + y,
                              (*(buf + xSize * i + j) & 0x0f) >> 1);
    }
  }

  return true;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void Image::beginDither() {
  m_ditherBuffer[0] =
      (uint8_t *)calloc((BMP_MAX_WIDTH + 2) * sizeof(uint16_t), 1);
  m_ditherBuffer[1] =
      (uint8_t *)calloc((BMP_MAX_WIDTH + 2) * sizeof(uint16_t), 1);
}

void Image::endDither() {
  free(m_ditherBuffer[0]);
  free(m_ditherBuffer[1]);
  m_ditherBuffer[0] = m_ditherBuffer[1] = nullptr;
}
