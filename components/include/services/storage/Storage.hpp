/**
 * @file Storage.hpp
 * @brief Persistent application storage backed by NVS.
 */

#pragma once

#include "esp_err.h"
#include "nvs.h"

#include <string>

class Storage {
public:
  Storage() = default;
  ~Storage();

  Storage(const Storage &) = delete;
  Storage &operator=(const Storage &) = delete;

  esp_err_t begin();

  bool isWifiProvisioned() const;
  esp_err_t setWifiProvisioned(bool provisioned);

  std::string pairingToken() const;
  esp_err_t setPairingToken(const std::string &token);
  bool hasPairingToken() const;

  std::string deviceId() const;
  esp_err_t setDeviceId(const std::string &deviceId);

  std::string deviceName() const;
  esp_err_t setDeviceName(const std::string &name);

private:
  std::string getString(const char *key) const;
  esp_err_t setString(const char *key, const std::string &value);

  nvs_handle_t m_handle = 0;
};
