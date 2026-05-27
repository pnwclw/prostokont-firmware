/**
 * @file DeviceIdentity.hpp
 * @brief Device identity service.
 */

#pragma once

#include "esp_err.h"

#include <string>

#include "services/storage/Storage.hpp"

class DeviceIdentity {
public:
  esp_err_t begin(Storage &storage);

  const std::string &deviceId() const { return m_deviceId; }
  const std::string &shortId() const { return m_shortId; }
  const std::string &hostname() const { return m_hostname; }
  const std::string &name() const { return m_name; }
  std::string shortIdUpper() const;

private:
  std::string m_deviceId;
  std::string m_shortId;
  std::string m_hostname;
  std::string m_name;
};
