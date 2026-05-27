/**
 * @file MdnsService.hpp
 * @brief mDNS service wrapper.
 */

#pragma once

#include "config/AppConfig.hpp"
#include "esp_err.h"

#include <stdint.h>

class DeviceIdentity;

class MdnsService {
public:
  esp_err_t begin(const char *hostname, const char *instanceName);
  esp_err_t begin(const DeviceIdentity &deviceIdentity);
  esp_err_t addHttpService(uint16_t port = config::kHttpPort);

private:
  bool m_started = false;
  bool m_httpServiceAdded = false;
};
