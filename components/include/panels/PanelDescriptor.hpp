/**
 * @file PanelDescriptor.hpp
 * @brief Unified compile-time contract for the selected display panel.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace prostokont {

enum PanelColorKind : uint8_t {
  PANEL_COLOR_KIND_BW = 0,
  PANEL_COLOR_KIND_GRAYSCALE = 1,
  PANEL_COLOR_KIND_PALETTE = 2,
};

struct PanelPaletteEntry {
  uint8_t id;
  uint32_t rgb;
  const char *name;
};

struct PanelDescriptor {
  const char *key;
  const char *manufacturer;
  const char *model;
  uint16_t widthMm;
  uint16_t widthPx;
  uint16_t heightMm;
  uint16_t heightPx;
  PanelColorKind colorKind;
  const PanelPaletteEntry *palette;
  size_t paletteSize;
  bool supportsPartialUpdate;
  bool supportsPackedFrame;
  bool colorImage;
  uint8_t defaultRotation;
  bool preferReducedDither;
  uint16_t accentColor;
  uint16_t errorColor;
  const char *colorScheme;
};

inline constexpr const char *panelColorKindName(PanelColorKind kind) {
  switch (kind) {
  case PANEL_COLOR_KIND_BW:
    return "bw";
  case PANEL_COLOR_KIND_GRAYSCALE:
    return "grayscale";
  case PANEL_COLOR_KIND_PALETTE:
    return "palette";
  }

  return "unknown";
}

} // namespace prostokont
