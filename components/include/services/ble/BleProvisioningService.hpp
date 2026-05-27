/**
 * @file BleProvisioningService.hpp
 * @brief BLE provisioning service for AccessorySetupKit-driven Wi-Fi setup.
 */

#pragma once

#include "esp_err.h"

class DeviceIdentity;
class DisplayService;
class MdnsService;
class Storage;
class WifiManager;

class BleProvisioningService {
public:
  BleProvisioningService(DeviceIdentity &deviceIdentity, Storage &storage,
                         WifiManager &wifiManager,
                         DisplayService &displayService,
                         MdnsService &mdnsService);

  BleProvisioningService(const BleProvisioningService &) = delete;
  BleProvisioningService &operator=(const BleProvisioningService &) = delete;

  esp_err_t begin();

private:
  DeviceIdentity &m_deviceIdentity;
  Storage &m_storage;
  WifiManager &m_wifiManager;
  DisplayService &m_displayService;
  MdnsService &m_mdnsService;
};
