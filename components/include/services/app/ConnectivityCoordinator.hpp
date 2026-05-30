/**
 * @file ConnectivityCoordinator.hpp
 * @brief Coordinates startup, onboarding, and connectivity fallback flows.
 */

#pragma once

#include "esp_err.h"

class BleDisplayService;
class BleProvisioningService;
class DeviceIdentity;
class DisplayService;
class MdnsService;
class Storage;
class WifiManager;

class ConnectivityCoordinator {
public:
  ConnectivityCoordinator(DeviceIdentity &deviceIdentity, Storage &storage,
                          WifiManager &wifiManager,
                          BleProvisioningService &bleProvisioningService,
                          BleDisplayService &bleDisplayService,
                          DisplayService &displayService,
                          MdnsService &mdnsService);

  esp_err_t begin();

private:
  struct MonitorContext {
    ConnectivityCoordinator *self;
  };

  static void monitorTask(void *arg);

  esp_err_t runStartupFlow();
  esp_err_t ensureSetupMode();
  esp_err_t ensureReady();
  void monitorLoop();

  DeviceIdentity &m_deviceIdentity;
  Storage &m_storage;
  WifiManager &m_wifiManager;
  BleProvisioningService &m_bleProvisioningService;
  BleDisplayService &m_bleDisplayService;
  DisplayService &m_displayService;
  MdnsService &m_mdnsService;
  MonitorContext m_monitorContext{this};
  bool m_monitorStarted = false;
  bool m_setupModeActive = false;
};
