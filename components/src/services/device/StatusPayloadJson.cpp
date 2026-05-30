/**
 * @file StatusPayloadJson.cpp
 * @brief Shared JSON helpers for HTTP and BLE status payloads.
 */

#include "services/device/StatusPayloadJson.hpp"

#include "cJSON.h"
#include "services/board/BoardProfile.hpp"
#include "services/device/DeviceIdentity.hpp"

#include <cstdio>
#include <utility>

namespace prostokont::status_json {

namespace {

void addFlagArray(cJSON *array, uint32_t flags,
                  const std::pair<uint32_t, const char *> *mapping,
                  size_t mappingCount) {
  for (size_t i = 0; i < mappingCount; ++i) {
    if ((flags & mapping[i].first) != 0)
      cJSON_AddItemToArray(array, cJSON_CreateString(mapping[i].second));
  }
}

void addPanelDescriptor(cJSON *parent, const BoardProfile &profile) {
  const auto &panel = profile.panel;
  cJSON *json = cJSON_AddObjectToObject(parent, "panel_info");
  addString(json, "key", panel.key);
  addString(json, "manufacturer", panel.manufacturer);
  addString(json, "model", panel.model);
  cJSON_AddNumberToObject(json, "width_mm", panel.widthMm);
  cJSON_AddNumberToObject(json, "width_px", panel.widthPx);
  cJSON_AddNumberToObject(json, "height_mm", panel.heightMm);
  cJSON_AddNumberToObject(json, "height_px", panel.heightPx);
  addString(json, "color_mode", panelColorKindName(panel.colorKind));
  addString(json, "color_scheme", panel.colorScheme);
  cJSON_AddBoolToObject(json, "supports_partial_update",
                        panel.supportsPartialUpdate);
  cJSON_AddBoolToObject(json, "supports_packed_frame",
                        panel.supportsPackedFrame);

  cJSON *palette = cJSON_AddArrayToObject(json, "palette");
  for (size_t i = 0; i < panel.paletteSize; ++i) {
    cJSON *entry = cJSON_CreateObject();
    cJSON_AddNumberToObject(entry, "id", panel.palette[i].id);
    addString(entry, "name", panel.palette[i].name);
    cJSON_AddNumberToObject(entry, "rgb", panel.palette[i].rgb);
    cJSON_AddItemToArray(palette, entry);
  }
}

void addBoardDescriptor(cJSON *parent, const BoardProfile &profile) {
  static constexpr std::pair<uint32_t, const char *> kProtocolNames[] = {
      {BOARD_PROTOCOL_SPI, "spi"},
      {BOARD_PROTOCOL_I2C, "i2c"},
      {BOARD_PROTOCOL_I2S, "i2s"},
      {BOARD_PROTOCOL_BLE, "ble"},
      {BOARD_PROTOCOL_WIFI_STA, "wifi-sta"},
      {BOARD_PROTOCOL_WIFI_SOFTAP, "wifi-softap"},
      {BOARD_PROTOCOL_HTTP, "http"},
      {BOARD_PROTOCOL_MDNS, "mdns"},
  };
  static constexpr std::pair<uint32_t, const char *> kSensorNames[] = {
      {BOARD_SENSOR_TOUCH, "touch"},
      {BOARD_SENSOR_RTC, "rtc"},
      {BOARD_SENSOR_FUEL_GAUGE, "fuel-gauge"},
      {BOARD_SENSOR_IMU, "imu"},
      {BOARD_SENSOR_AMBIENT, "ambient"},
      {BOARD_SENSOR_ENVIRONMENTAL, "environmental"},
  };
  static constexpr std::pair<uint32_t, const char *> kPowerNames[] = {
      {BOARD_POWER_USB, "usb"},
      {BOARD_POWER_BATTERY, "battery"},
      {BOARD_POWER_FUEL_GAUGE, "fuel-gauge"},
      {BOARD_POWER_FRONTLIGHT, "frontlight"},
      {BOARD_POWER_PANEL_GATE, "panel-gate"},
  };

  const auto &board = profile.board;
  cJSON *json = cJSON_AddObjectToObject(parent, "board_info");
  addString(json, "key", board.key);
  addString(json, "name", board.name);

  cJSON *protocols = cJSON_AddArrayToObject(json, "communication_protocols");
  addFlagArray(protocols, board.communicationProtocols, kProtocolNames,
               sizeof(kProtocolNames) / sizeof(kProtocolNames[0]));

  cJSON *sensors = cJSON_AddArrayToObject(json, "sensors");
  addFlagArray(sensors, board.sensors, kSensorNames,
               sizeof(kSensorNames) / sizeof(kSensorNames[0]));

  cJSON *power = cJSON_AddArrayToObject(json, "power");
  addFlagArray(power, board.powerFeatures, kPowerNames,
               sizeof(kPowerNames) / sizeof(kPowerNames[0]));
}

void addFirmwareDescriptor(cJSON *parent, const BoardProfile &profile) {
  const auto &firmware = profile.firmware;
  cJSON *json = cJSON_AddObjectToObject(parent, "firmware_info");
  addString(json, "product_name", firmware.productName);
  addString(json, "model", firmware.model);
  addString(json, "version", firmware.version);

  cJSON *boards = cJSON_AddArrayToObject(json, "supported_boards");
  for (size_t i = 0; i < firmware.supportedBoardCount; ++i)
    cJSON_AddItemToArray(boards, cJSON_CreateString(firmware.supportedBoards[i]));

  cJSON *panels = cJSON_AddArrayToObject(json, "supported_panels");
  for (size_t i = 0; i < firmware.supportedPanelCount; ++i)
    cJSON_AddItemToArray(panels, cJSON_CreateString(firmware.supportedPanels[i]));
}

} // namespace

void addString(cJSON *json, const char *key, const char *value) {
  cJSON_AddStringToObject(json, key, value != nullptr ? value : "");
}

void addDeviceIdentity(cJSON *json, const DeviceIdentity &deviceIdentity) {
  addString(json, "device_id", deviceIdentity.deviceId().c_str());
  addString(json, "name", deviceIdentity.name().c_str());
  addString(json, "hostname", deviceIdentity.hostname().c_str());
}

void addApiBase(cJSON *json, const DeviceIdentity &deviceIdentity,
                const char *apiBasePath) {
  char apiBase[128];
  std::snprintf(apiBase, sizeof(apiBase), "http://%s.local%s",
                deviceIdentity.hostname().c_str(),
                apiBasePath != nullptr ? apiBasePath : "");
  addString(json, "api", apiBase);
}

void addDefaultCapabilities(cJSON *json) {
  cJSON *capabilities = cJSON_AddArrayToObject(json, "capabilities");
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("wifi-provisioning"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("ble-display-v1"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("image-upload"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("image-clear"));
}

void addFirmwareSummary(cJSON *json, const BoardProfile &profile) {
  addString(json, "model", profile.firmware.model);
  addString(json, "board", profile.key);
  addString(json, "firmware_version", profile.firmware.version);
}

void addConnectivity(cJSON *json, bool wifiProvisioned, bool staConnected) {
  cJSON_AddBoolToObject(json, "wifi_provisioned", wifiProvisioned);
  cJSON_AddBoolToObject(json, "sta_connected", staConnected);
}

void addDisplaySummary(cJSON *json, const BoardProfile &profile) {
  cJSON_AddNumberToObject(json, "display_width", profile.width);
  cJSON_AddNumberToObject(json, "display_height", profile.height);
  cJSON_AddBoolToObject(json, "packed_frame_supported",
                        profile.packedFrameSupported);
  addString(json, "color_scheme", profile.colorScheme);
}

void addHardwareContract(cJSON *json, const BoardProfile &profile) {
  addPanelDescriptor(json, profile);
  addBoardDescriptor(json, profile);
  addFirmwareDescriptor(json, profile);
}

void addState(cJSON *json, const char *state) { addString(json, "state", state); }

void addLastError(cJSON *json, const char *lastError) {
  if (lastError != nullptr && lastError[0] != '\0')
    addString(json, "last_error", lastError);
}

void addDisplayTransferStatus(cJSON *json, const char *protocol,
                              size_t bytesExpected, size_t bytesReceived,
                              const char *format) {
  addString(json, "protocol", protocol);
  cJSON_AddNumberToObject(json, "bytes_expected", bytesExpected);
  cJSON_AddNumberToObject(json, "bytes_received", bytesReceived);
  addString(json, "format", format);
}

void addRuntimeStatus(cJSON *json, const DeviceIdentity &deviceIdentity,
                      const BoardProfile &profile, bool wifiProvisioned,
                      bool staConnected) {
  addDeviceIdentity(json, deviceIdentity);
  addFirmwareSummary(json, profile);
  addConnectivity(json, wifiProvisioned, staConnected);
  addDisplaySummary(json, profile);
  addHardwareContract(json, profile);
}

std::string printUnformatted(const cJSON *json) {
  if (json == nullptr)
    return {};

  char *rendered = cJSON_PrintUnformatted(json);
  if (rendered == nullptr)
    return {};

  std::string out(rendered);
  cJSON_free(rendered);
  return out;
}

} // namespace prostokont::status_json
