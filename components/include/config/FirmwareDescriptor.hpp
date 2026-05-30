/**
 * @file FirmwareDescriptor.hpp
 * @brief Unified compile-time contract for the selected firmware build.
 */

#pragma once

#include <cstddef>

namespace prostokont {

struct FirmwareDescriptor {
  const char *productName;
  const char *model;
  const char *version;
  const char *const *supportedBoards;
  size_t supportedBoardCount;
  const char *const *supportedPanels;
  size_t supportedPanelCount;
};

} // namespace prostokont
