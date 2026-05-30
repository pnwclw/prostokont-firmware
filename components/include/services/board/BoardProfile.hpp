/**
 * @file BoardProfile.hpp
 * @brief Compile-time board metadata exposed to services and clients.
 */

#pragma once

#include "panels/PanelFactory.hpp"

#include <cstdint>
#include <cstddef>

enum DisplayInputFormat : uint32_t {
  kDisplayInputFormatBmp = 1u << 0,
  kDisplayInputFormatJpeg = 1u << 1,
  kDisplayInputFormatPng = 1u << 2,
  kDisplayInputFormatPackedFrame = 1u << 3,
};

inline constexpr uint32_t kDecodedDisplayInputFormats =
    kDisplayInputFormatBmp | kDisplayInputFormatJpeg | kDisplayInputFormatPng;

struct BoardProfile {
  const char *key;
  const char *name;
  int width;
  int height;
  bool packedFrameSupported;
  const char *colorScheme;
  uint32_t displayInputFormats;
  uint8_t defaultRotation;
  bool colorImage;
  bool preferReducedDither;
  uint16_t accentColor;
  uint16_t errorColor;
  prostokont::PanelDescriptor panel;
  prostokont::BoardDescriptor board;
  prostokont::FirmwareDescriptor firmware;
};

inline const BoardProfile &currentBoardProfile() {
  constexpr prostokont::PanelDescriptor panel =
      prostokont::currentPanelDescriptor();
  constexpr prostokont::BoardDescriptor board =
      prostokont::currentBoardDescriptor();
  constexpr prostokont::FirmwareDescriptor firmware =
      prostokont::currentFirmwareDescriptor();
  static constexpr BoardProfile profile = {
      .key = board.key,
      .name = board.name,
      .width = panel.widthPx,
      .height = panel.heightPx,
      .packedFrameSupported = panel.supportsPackedFrame,
      .colorScheme = panel.colorScheme,
      .displayInputFormats =
          kDecodedDisplayInputFormats |
          (panel.supportsPackedFrame
               ? static_cast<uint32_t>(kDisplayInputFormatPackedFrame)
               : 0u),
      .defaultRotation = panel.defaultRotation,
      .colorImage = panel.colorImage,
      .preferReducedDither = panel.preferReducedDither,
      .accentColor = panel.accentColor,
      .errorColor = panel.errorColor,
      .panel = panel,
      .board = board,
      .firmware = firmware,
  };
  return profile;
}

inline constexpr bool supportsDisplayInputFormat(const BoardProfile &profile,
                                                 DisplayInputFormat format) {
  return (profile.displayInputFormats & format) != 0;
}

inline size_t currentPackedFrameBytes() {
  if (!currentBoardProfile().packedFrameSupported)
    return 0;

  return static_cast<size_t>(currentBoardProfile().width) *
         static_cast<size_t>(currentBoardProfile().height) / 2;
}
