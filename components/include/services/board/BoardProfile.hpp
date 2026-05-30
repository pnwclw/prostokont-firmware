/**
 * @file BoardProfile.hpp
 * @brief Compile-time board metadata exposed to services and clients.
 */

#pragma once

#include "Display.h"

#include <cstddef>

struct BoardProfile {
  const char *key;
  const char *name;
  int width;
  int height;
  bool packedFrameSupported;
  const char *colorScheme;
};

inline const BoardProfile &currentBoardProfile() {
#if defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
  static constexpr BoardProfile profile = {
      .key = "waveshare-13.3-e6",
      .name = "Waveshare 13.3 inch E6",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = true,
      .colorScheme = "6-color",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
  static constexpr BoardProfile profile = {
      .key = "inkplate-13",
      .name = "Inkplate 13",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = true,
      .colorScheme = "7-color",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
  static constexpr BoardProfile profile = {
      .key = "inkplate-6-color",
      .name = "Inkplate 6 Color",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = true,
      .colorScheme = "7-color",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE2)
  static constexpr BoardProfile profile = {
      .key = "inkplate-2",
      .name = "Inkplate 2",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = true,
      .colorScheme = "3-color",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE10)
  static constexpr BoardProfile profile = {
      .key = "inkplate-10",
      .name = "Inkplate 10",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "monochrome",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE6)
  static constexpr BoardProfile profile = {
      .key = "inkplate-6",
      .name = "Inkplate 6",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "monochrome",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
  static constexpr BoardProfile profile = {
      .key = "inkplate-6-flick",
      .name = "Inkplate 6 Flick",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "monochrome",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE5)
  static constexpr BoardProfile profile = {
      .key = "inkplate-5",
      .name = "Inkplate 5",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "monochrome",
  };
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE4)
  static constexpr BoardProfile profile = {
      .key = "inkplate-4",
      .name = "Inkplate 4",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "monochrome",
  };
#else
  static constexpr BoardProfile profile = {
      .key = "unknown",
      .name = "Unknown board",
      .width = E_INK_WIDTH,
      .height = E_INK_HEIGHT,
      .packedFrameSupported = false,
      .colorScheme = "unknown",
  };
#endif
  return profile;
}

inline size_t currentPackedFrameBytes() {
  return static_cast<size_t>(currentBoardProfile().width) *
         static_cast<size_t>(currentBoardProfile().height) / 2;
}
