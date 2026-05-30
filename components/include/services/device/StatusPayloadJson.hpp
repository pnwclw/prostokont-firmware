/**
 * @file StatusPayloadJson.hpp
 * @brief Shared JSON helpers for HTTP and BLE status payloads.
 */

#pragma once

#include <cstddef>
#include <string>

struct cJSON;

class DeviceIdentity;
struct BoardProfile;

namespace prostokont::status_json {

void addString(cJSON *json, const char *key, const char *value);
void addDeviceIdentity(cJSON *json, const DeviceIdentity &deviceIdentity);
void addApiBase(cJSON *json, const DeviceIdentity &deviceIdentity,
                const char *apiBasePath);
void addDefaultCapabilities(cJSON *json);
void addFirmwareSummary(cJSON *json, const BoardProfile &profile);
void addConnectivity(cJSON *json, bool wifiProvisioned, bool staConnected);
void addDisplaySummary(cJSON *json, const BoardProfile &profile);
void addHardwareContract(cJSON *json, const BoardProfile &profile);
void addState(cJSON *json, const char *state);
void addLastError(cJSON *json, const char *lastError);
void addDisplayTransferStatus(cJSON *json, const char *protocol,
                              size_t bytesExpected, size_t bytesReceived,
                              const char *format);
void addRuntimeStatus(cJSON *json, const DeviceIdentity &deviceIdentity,
                      const BoardProfile &profile, bool wifiProvisioned,
                      bool staConnected);
std::string printUnformatted(const cJSON *json);

} // namespace prostokont::status_json
