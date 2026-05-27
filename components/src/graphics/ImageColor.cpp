/**
 * @file   ImageColor.cpp
 * @author Fran Fodor for Soldered
 * @brief  Drawing color images.
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
#include "math.h"
#include "string.h"

#include "ImageColor.h"
#include "Display.h"

static const char *TAG = "ImageColor";

namespace {

struct RgbColor {
  int16_t r;
  int16_t g;
  int16_t b;
};

inline int16_t clampChannel(int value) {
  if (value < 0)
    return 0;
  if (value > 255)
    return 255;
  return static_cast<int16_t>(value);
}

inline int16_t computeLuma(int16_t r, int16_t g, int16_t b) {
  return static_cast<int16_t>((54 * r + 183 * g + 19 * b) / 256);
}

inline RgbColor tuneSourceColor(int16_t r, int16_t g, int16_t b) {
#if defined(CONFIG_INKPLATE_BOARD_INKPLATE13) ||                              \
    defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  constexpr int kContrastPct = 86;
  constexpr int kBrightnessOffset = 10;
  constexpr int kSaturationPct = 122;

  int tr = 128 + ((r - 128) * kContrastPct) / 100 + kBrightnessOffset;
  int tg = 128 + ((g - 128) * kContrastPct) / 100 + kBrightnessOffset;
  int tb = 128 + ((b - 128) * kContrastPct) / 100 + kBrightnessOffset;

  const int16_t luma =
      computeLuma(clampChannel(tr), clampChannel(tg), clampChannel(tb));
  tr = luma + ((tr - luma) * kSaturationPct) / 100;
  tg = luma + ((tg - luma) * kSaturationPct) / 100;
  tb = luma + ((tb - luma) * kSaturationPct) / 100;

  return {clampChannel(tr), clampChannel(tg), clampChannel(tb)};
#else
  return {r, g, b};
#endif
}

inline int32_t paletteDistance(uint32_t paletteColor, int16_t r, int16_t g,
                               int16_t b) {
  const int16_t pr = static_cast<int16_t>(RED8(paletteColor));
  const int16_t pg = static_cast<int16_t>(GREEN8(paletteColor));
  const int16_t pb = static_cast<int16_t>(BLUE8(paletteColor));
  const int16_t dr = r - pr;
  const int16_t dg = g - pg;
  const int16_t db = b - pb;
  const int16_t dl = computeLuma(r, g, b) - computeLuma(pr, pg, pb);

  return (3 * dr * dr) + (5 * dg * dg) + (2 * db * db) + (4 * dl * dl);
}

uint8_t findClosestPaletteEntryImpl(const uint32_t *palette, uint8_t paletteSize,
                                    int16_t r, int16_t g, int16_t b) {
  int32_t minDistance = INT32_MAX;
  uint8_t closest = 0;

  for (uint8_t i = 0; i < paletteSize; ++i) {
    const int32_t currentDistance = paletteDistance(palette[i], r, g, b);
    if (currentDistance < minDistance) {
      minDistance = currentDistance;
      closest = i;
    }
  }

  return closest;
}

} // namespace

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

ImageColor::ImageColor(Display *inkplate)
    : m_inkplate(inkplate), m_bmp(inkplate), m_jpeg(inkplate), m_png(inkplate) {
  for (int i = 0; i < DITHER_ROW_COUNT; ++i)
    m_ditherR[i] = m_ditherG[i] = m_ditherB[i] = nullptr;
  m_rowIdx = 0;
  for (uint8_t i = 0; i < 7; ++i)
    m_paletteColorCode[i] = i;

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
  palletteSize = 7;
  pallete[0] = 0x000000;
  pallete[1] = 0xFFFFFF;
  pallete[2] = 0x00FF00;
  pallete[3] = 0x0000FF;
  pallete[4] = 0xFF0000;
  pallete[5] = 0xFFFF00;
  pallete[6] = 0xFF8000;
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13) ||                            \
    defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  palletteSize = 6;
  pallete[0] = 0x000000;
  pallete[1] = 0xFFFFFF;
  pallete[2] = 0xFFFF00;
  pallete[3] = 0xFF0000;
  pallete[4] = 0x0000FF;
  pallete[5] = 0x00FF00;
  m_paletteColorCode[4] = 5;
  m_paletteColorCode[5] = 6;
#else
  palletteSize = 3;
  pallete[0] = 0xFFFFFF;
  pallete[1] = 0x000000;
  pallete[2] = 0xFF0000;
#endif
}

uint8_t ImageColor::findClosestPalette(int16_t r, int16_t g, int16_t b) {
  return m_paletteColorCode[findClosestPaletteEntry(r, g, b)];
}

uint8_t ImageColor::findClosestPaletteEntry(int16_t r, int16_t g, int16_t b) {
  const RgbColor tuned = tuneSourceColor(r, g, b);
  return findClosestPaletteEntryImpl(pallete, palletteSize, tuned.r, tuned.g,
                                     tuned.b);
}

void ImageColor::setDitherKernel(DitherKernel kernel) {
  if ((uint8_t)kernel < DITHER_KERNEL_COUNT)
    m_currentKernel = &DITHER_KERNELS[(uint8_t)kernel];
}

uint8_t ImageColor::getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i,
                                     int w) {
  if (i < 0 || i >= BMP_MAX_WIDTH)
    return findClosestPalette(r, g, b);

  const int rowIdx = m_rowIdx & DITHER_ROW_MASK;
  int16_t *rowR = m_ditherR[rowIdx];
  int16_t *rowG = m_ditherG[rowIdx];
  int16_t *rowB = m_ditherB[rowIdx];

  if (!rowR || !rowG || !rowB)
    return findClosestPalette(r, g, b);

  const RgbColor tuned = tuneSourceColor(r, g, b);

  int16_t er = tuned.r + rowR[i];
  int16_t eg = tuned.g + rowG[i];
  int16_t eb = tuned.b + rowB[i];

  rowR[i] = 0;
  rowG[i] = 0;
  rowB[i] = 0;

  if (er < 0)
    er = 0;
  else if (er > 255)
    er = 255;
  if (eg < 0)
    eg = 0;
  else if (eg > 255)
    eg = 255;
  if (eb < 0)
    eb = 0;
  else if (eb > 255)
    eb = 255;

  int closest = findClosestPaletteEntryImpl(pallete, palletteSize, er, eg, eb);

  int32_t rErr = er - (int32_t)RED8(pallete[closest]);
  int32_t gErr = eg - (int32_t)GREEN8(pallete[closest]);
  int32_t bErr = eb - (int32_t)BLUE8(pallete[closest]);

  const DitherKernelDef *k = m_currentKernel;
  const int minOffset = (i < (int)k->x) ? -i : -(int)k->x;
  const int maxOffset = ((int)k->width - k->x - 1 < w - 1 - i)
                            ? (int)k->width - k->x - 1
                            : w - 1 - i;

  for (int ky = 0; ky < k->height; ++ky) {
    const int nextRowIdx = (rowIdx + ky) & DITHER_ROW_MASK;
    int16_t *nextRowR = m_ditherR[nextRowIdx];
    int16_t *nextRowG = m_ditherG[nextRowIdx];
    int16_t *nextRowB = m_ditherB[nextRowIdx];
    for (int l = minOffset; l <= maxOffset; ++l) {
      const int weight = k->data[ky * k->width + (l + k->x)];
      if (!weight)
        continue;
      const int idx = i + l;
      nextRowR[idx] += (int16_t)((weight * rErr) / k->coef);
      nextRowG[idx] += (int16_t)((weight * gErr) / k->coef);
      nextRowB[idx] += (int16_t)((weight * bErr) / k->coef);
    }
  }

  return m_paletteColorCode[closest];
}

void ImageColor::ditherSwap(int w) {
  m_rowIdx = (m_rowIdx + 1) & DITHER_ROW_MASK;
}

bool ImageColor::draw(uint8_t *buf, int x, int y, bool dither, bool invert) {
  if (buf[0] != 0x42 || buf[1] != 0x4D) {
    ESP_LOGE(TAG, "Unrecognised image format (supply length for JPEG/PNG)");
    return false;
  }
  if (dither)
    beginDither();
  bool result = m_bmp.draw(buf, x, y, dither, invert);
  if (dither)
    endDither();
  return result;
}

bool ImageColor::draw(uint8_t *buf, int32_t len, int x, int y, bool dither,
                      bool invert) {
  if (dither)
    beginDither();

  bool result = false;
  if (buf[0] == 0xFF && buf[1] == 0xD8)
    result = m_jpeg.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47)
    result = m_png.draw(buf, len, x, y, dither, invert);
  else if (buf[0] == 0x42 && buf[1] == 0x4D)
    result = m_bmp.draw(buf, x, y, dither, invert);
  else
    ESP_LOGE(TAG, "Unrecognised image format");

  if (dither)
    endDither();
  return result;
}

bool ImageColor::draw(const char *src, int x, int y, bool dither, bool invert) {
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
  }
#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
  else {
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
#else
  else {
    ESP_LOGE(TAG, "SD card not supported on this board.");
  }
#endif

  bool result = draw(buf, len, x, y, dither, invert);
  free(buf);
  return result;
}

bool ImageColor::draw(const uint8_t *buf, int x, int y, int w, int h, int c) {
  int64_t lastYieldUs = esp_timer_get_time();

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) ||                           \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE13) ||                                \
    defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
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
#else
  uint16_t scaledW = ceil(w / 4.0);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < scaledW; j++) {
      m_inkplate->drawPixel(4 * j + x + 0, i + y,
                            (buf[scaledW * i + j] & 0xC0) >> 6);
      m_inkplate->drawPixel(4 * j + x + 1, i + y,
                            (buf[scaledW * i + j] & 0x30) >> 4);
      m_inkplate->drawPixel(4 * j + x + 2, i + y,
                            (buf[scaledW * i + j] & 0x0C) >> 2);
      m_inkplate->drawPixel(4 * j + x + 3, i + y,
                            (buf[scaledW * i + j] & 0x03));
    }
  }
#endif

  return true;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void ImageColor::beginDither() {
  m_rowIdx = 0;
  for (int i = 0; i < DITHER_ROW_COUNT; ++i) {
    m_ditherR[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherG[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
    m_ditherB[i] = (int16_t *)calloc(BMP_MAX_WIDTH + 2, sizeof(int16_t));
  }
}

void ImageColor::endDither() {
  for (int i = 0; i < DITHER_ROW_COUNT; ++i) {
    free(m_ditherR[i]);
    m_ditherR[i] = nullptr;
    free(m_ditherG[i]);
    m_ditherG[i] = nullptr;
    free(m_ditherB[i]);
    m_ditherB[i] = nullptr;
  }
}
