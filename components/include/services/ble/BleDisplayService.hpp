/**
 * @file BleDisplayService.hpp
 * @brief BLE display transport for image upload and display control.
 */

#pragma once

#include "esp_err.h"

class DeviceIdentity;
class DisplayService;
class Storage;
class WifiManager;

class BleDisplayService {
public:
  BleDisplayService(DeviceIdentity &deviceIdentity, Storage &storage,
                    WifiManager &wifiManager, DisplayService &displayService);

  BleDisplayService(const BleDisplayService &) = delete;
  BleDisplayService &operator=(const BleDisplayService &) = delete;

  esp_err_t begin();

private:
  DeviceIdentity &m_deviceIdentity;
  Storage &m_storage;
  WifiManager &m_wifiManager;
  DisplayService &m_displayService;
};
