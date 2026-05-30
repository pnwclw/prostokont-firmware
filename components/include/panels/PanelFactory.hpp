/**
 * @file PanelFactory.hpp
 * @brief Selected-panel factory and descriptor accessors.
 */

#pragma once

#include "config/AppConfig.hpp"
#include "config/FirmwareDescriptor.hpp"
#include "panels/PanelDescriptor.hpp"
#include "services/board/BoardDescriptor.hpp"
#include "config/SelectedBoard.hpp"

namespace prostokont {

namespace detail {

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2)
inline constexpr PanelPaletteEntry kSelectedPanelPalette[] = {
    {INKPLATE2_WHITE, 0xFFFFFFu, "white"},
    {INKPLATE2_BLACK, 0x000000u, "black"},
    {INKPLATE2_RED, 0xFF0000u, "red"},
};
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
inline constexpr PanelPaletteEntry kSelectedPanelPalette[] = {
    {0u, 0x000000u, "black"},
    {1u, 0xFFFFFFu, "white"},
    {2u, 0x00FF00u, "green"},
    {3u, 0x0000FFu, "blue"},
    {4u, 0xFF0000u, "red"},
    {5u, 0xFFFF00u, "yellow"},
    {6u, 0xFFA500u, "orange"},
};
#elif defined(CONFIG_INKPLATE_BOARD_INKPLATE13)
inline constexpr PanelPaletteEntry kSelectedPanelPalette[] = {
    {INKPLATE_BLACK, 0x000000u, "black"},
    {INKPLATE_WHITE, 0xFFFFFFu, "white"},
    {INKPLATE_YELLOW, 0xFFFF00u, "yellow"},
    {INKPLATE_RED, 0xFF0000u, "red"},
    {INKPLATE_BLUE, 0x0000FFu, "blue"},
    {INKPLATE_GREEN, 0x00FF00u, "green"},
};
#elif defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
inline constexpr PanelPaletteEntry kSelectedPanelPalette[] = {
    {WAVESHARE13_BLACK, 0x000000u, "black"},
    {WAVESHARE13_WHITE, 0xFFFFFFu, "white"},
    {WAVESHARE13_YELLOW, 0xFFFF00u, "yellow"},
    {WAVESHARE13_RED, 0xFF0000u, "red"},
    {WAVESHARE13_BLUE, 0x0000FFu, "blue"},
    {WAVESHARE13_GREEN, 0x00FF00u, "green"},
};
#else
inline constexpr PanelPaletteEntry kSelectedPanelPalette[] = {};
#endif

inline constexpr const char *kSupportedBoards[] = {PROSTOKONT_BOARD_KEY};
inline constexpr const char *kSupportedPanels[] = {PROSTOKONT_PANEL_KEY};

} // namespace detail

class PanelFactory {
public:
  using PanelType = PROSTOKONT_BOARD_PANEL_CLASS;

  static PanelType create() { return PanelType(); }

  static constexpr PanelDescriptor descriptor() {
    return {
        .key = PROSTOKONT_PANEL_KEY,
        .manufacturer = PROSTOKONT_PANEL_MANUFACTURER,
        .model = PROSTOKONT_PANEL_MODEL,
        .widthMm = PROSTOKONT_PANEL_WIDTH_MM,
        .widthPx = E_INK_WIDTH,
        .heightMm = PROSTOKONT_PANEL_HEIGHT_MM,
        .heightPx = E_INK_HEIGHT,
        .colorKind =
            static_cast<PanelColorKind>(PROSTOKONT_PANEL_COLOR_KIND),
        .palette = detail::kSelectedPanelPalette,
        .paletteSize = sizeof(detail::kSelectedPanelPalette) /
                       sizeof(detail::kSelectedPanelPalette[0]),
        .supportsPartialUpdate = PROSTOKONT_PANEL_PARTIAL_UPDATE_SUPPORTED,
        .supportsPackedFrame = PROSTOKONT_BOARD_PACKED_FRAME_SUPPORTED,
        .colorImage = PROSTOKONT_BOARD_COLOR_IMAGE,
        .defaultRotation = PROSTOKONT_BOARD_DEFAULT_ROTATION,
        .preferReducedDither = PROSTOKONT_BOARD_PREFER_REDUCED_DITHER,
        .accentColor = PROSTOKONT_BOARD_ACCENT_COLOR,
        .errorColor = PROSTOKONT_BOARD_ERROR_COLOR,
        .colorScheme = PROSTOKONT_BOARD_COLOR_SCHEME,
    };
  }
};

inline constexpr BoardDescriptor currentBoardDescriptor() {
  return {
      .key = PROSTOKONT_BOARD_KEY,
      .name = PROSTOKONT_BOARD_NAME,
      .communicationProtocols = PROSTOKONT_BOARD_COMMUNICATION_PROTOCOLS,
      .sensors = PROSTOKONT_BOARD_SENSORS,
      .powerFeatures = PROSTOKONT_BOARD_POWER_FEATURES,
  };
}

inline constexpr PanelDescriptor currentPanelDescriptor() {
  return PanelFactory::descriptor();
}

inline constexpr FirmwareDescriptor currentFirmwareDescriptor() {
  return {
      .productName = config::kProductName,
      .model = PROSTOKONT_FIRMWARE_MODEL,
      .version = config::kFirmwareVersion,
      .supportedBoards = detail::kSupportedBoards,
      .supportedBoardCount =
          sizeof(detail::kSupportedBoards) / sizeof(detail::kSupportedBoards[0]),
      .supportedPanels = detail::kSupportedPanels,
      .supportedPanelCount =
          sizeof(detail::kSupportedPanels) / sizeof(detail::kSupportedPanels[0]),
  };
}

} // namespace prostokont
