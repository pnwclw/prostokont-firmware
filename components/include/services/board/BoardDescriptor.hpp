/**
 * @file BoardDescriptor.hpp
 * @brief Unified compile-time contract for the selected board.
 */

#pragma once

#include <cstdint>

namespace prostokont {

enum BoardCommunicationProtocol : uint32_t {
  BOARD_PROTOCOL_SPI = 1u << 0,
  BOARD_PROTOCOL_I2C = 1u << 1,
  BOARD_PROTOCOL_I2S = 1u << 2,
  BOARD_PROTOCOL_BLE = 1u << 3,
  BOARD_PROTOCOL_WIFI_STA = 1u << 4,
  BOARD_PROTOCOL_WIFI_SOFTAP = 1u << 5,
  BOARD_PROTOCOL_HTTP = 1u << 6,
  BOARD_PROTOCOL_MDNS = 1u << 7,
};

enum BoardSensor : uint32_t {
  BOARD_SENSOR_TOUCH = 1u << 0,
  BOARD_SENSOR_RTC = 1u << 1,
  BOARD_SENSOR_FUEL_GAUGE = 1u << 2,
  BOARD_SENSOR_IMU = 1u << 3,
  BOARD_SENSOR_AMBIENT = 1u << 4,
  BOARD_SENSOR_ENVIRONMENTAL = 1u << 5,
};

enum BoardPowerFeature : uint32_t {
  BOARD_POWER_USB = 1u << 0,
  BOARD_POWER_BATTERY = 1u << 1,
  BOARD_POWER_FUEL_GAUGE = 1u << 2,
  BOARD_POWER_FRONTLIGHT = 1u << 3,
  BOARD_POWER_PANEL_GATE = 1u << 4,
};

struct BoardDescriptor {
  const char *key;
  const char *name;
  uint32_t communicationProtocols;
  uint32_t sensors;
  uint32_t powerFeatures;
};

inline constexpr bool hasBoardProtocol(const BoardDescriptor &descriptor,
                                       BoardCommunicationProtocol protocol) {
  return (descriptor.communicationProtocols & protocol) != 0;
}

inline constexpr bool hasBoardSensor(const BoardDescriptor &descriptor,
                                     BoardSensor sensor) {
  return (descriptor.sensors & sensor) != 0;
}

inline constexpr bool hasBoardPowerFeature(const BoardDescriptor &descriptor,
                                           BoardPowerFeature feature) {
  return (descriptor.powerFeatures & feature) != 0;
}

} // namespace prostokont
