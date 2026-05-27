/**
 * @file DeviceIdentity.cpp
 * @brief Device identity service.
 */

#include "services/device/DeviceIdentity.hpp"

#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_mac.h"

#include <array>
#include <cctype>
#include <cstring>

namespace {

bool startsWith(const std::string &value, const char *prefix) {
  return value.rfind(prefix, 0) == 0;
}

std::string replacePrefix(const std::string &value, const char *oldPrefix,
                          const char *newPrefix) {
  return std::string(newPrefix) + value.substr(std::strlen(oldPrefix));
}

} // namespace

static const char *TAG = "ESP_DEVICE_IDENTITY";

static std::string makeHex(const uint8_t *bytes, size_t len, bool upper) {
  constexpr const char *LOWER_DIGITS = "0123456789abcdef";
  constexpr const char *UPPER_DIGITS = "0123456789ABCDEF";
  const char *digits = upper ? UPPER_DIGITS : LOWER_DIGITS;

  std::string out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    out.push_back(digits[(bytes[i] >> 4) & 0x0f]);
    out.push_back(digits[bytes[i] & 0x0f]);
  }
  return out;
}

esp_err_t DeviceIdentity::begin(Storage &storage) {
  std::array<uint8_t, 6> mac = {};
  ESP_RETURN_ON_ERROR(esp_efuse_mac_get_default(mac.data()), TAG,
                      "Failed to read default MAC");

  m_shortId = makeHex(mac.data() + 4, 2, false);
  m_hostname = std::string(config::kHostnamePrefix) + m_shortId;

  m_deviceId = storage.deviceId();
  if (m_deviceId.empty()) {
    m_deviceId = std::string(config::kDeviceIdPrefix) +
                 makeHex(mac.data() + 2, 4, true);
    ESP_RETURN_ON_ERROR(storage.setDeviceId(m_deviceId), TAG,
                        "Failed to persist device ID");
  } else if (startsWith(m_deviceId, config::kLegacyDeviceIdPrefix)) {
    m_deviceId = replacePrefix(m_deviceId, config::kLegacyDeviceIdPrefix,
                               config::kDeviceIdPrefix);
    ESP_RETURN_ON_ERROR(storage.setDeviceId(m_deviceId), TAG,
                        "Failed to migrate device ID");
  }

  m_name = storage.deviceName();
  if (m_name.empty()) {
    m_name = std::string(config::kProductName) + " " + shortIdUpper();
    ESP_RETURN_ON_ERROR(storage.setDeviceName(m_name), TAG,
                        "Failed to persist device name");
  } else if (startsWith(m_name, config::kLegacyProductName)) {
    m_name = replacePrefix(m_name, config::kLegacyProductName,
                           config::kProductName);
    ESP_RETURN_ON_ERROR(storage.setDeviceName(m_name), TAG,
                        "Failed to migrate device name");
  }

  ESP_LOGI(TAG, "Device initialized: id=%s hostname=%s", m_deviceId.c_str(),
           m_hostname.c_str());
  return ESP_OK;
}

std::string DeviceIdentity::shortIdUpper() const {
  std::string out = m_shortId;
  for (char &ch : out)
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));

  return out;
}
