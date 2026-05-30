/**
 * @file ConnectivityCoordinator.cpp
 * @brief Coordinates startup, onboarding, and connectivity fallback flows.
 */

#include "services/app/ConnectivityCoordinator.hpp"

#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "services/ble/BleDisplayService.hpp"
#include "services/ble/BleProvisioningService.hpp"
#include "services/device/DeviceIdentity.hpp"
#include "services/display/DisplayService.hpp"
#include "services/network/MdnsService.hpp"
#include "services/storage/Storage.hpp"
#include "services/wifi/WifiManager.hpp"

#include <string>

namespace {

constexpr const char *TAG = "CONNECTIVITY_COORD";

} // namespace

ConnectivityCoordinator::ConnectivityCoordinator(
    DeviceIdentity &deviceIdentity, Storage &storage, WifiManager &wifiManager,
    BleProvisioningService &bleProvisioningService,
    BleDisplayService &bleDisplayService, DisplayService &displayService,
    MdnsService &mdnsService)
    : m_deviceIdentity(deviceIdentity),
      m_storage(storage),
      m_wifiManager(wifiManager),
      m_bleProvisioningService(bleProvisioningService),
      m_bleDisplayService(bleDisplayService),
      m_displayService(displayService),
      m_mdnsService(mdnsService) {}

esp_err_t ConnectivityCoordinator::begin() {
  ESP_RETURN_ON_ERROR(m_bleProvisioningService.begin(), TAG,
                      "Failed to start BLE provisioning service");
  ESP_RETURN_ON_ERROR(m_bleDisplayService.begin(), TAG,
                      "Failed to start BLE display service");
  ESP_RETURN_ON_ERROR(runStartupFlow(), TAG, "Startup flow failed");

  if (!m_monitorStarted) {
    if (xTaskCreate(monitorTask, "wifi_setup_monitor", 4096, &m_monitorContext,
                    4, nullptr) == pdPASS) {
      m_monitorStarted = true;
    } else {
      return ESP_ERR_NO_MEM;
    }
  }

  return ESP_OK;
}

void ConnectivityCoordinator::monitorTask(void *arg) {
  auto *ctx = static_cast<MonitorContext *>(arg);
  ctx->self->monitorLoop();
}

esp_err_t ConnectivityCoordinator::runStartupFlow() {
  if (!m_storage.isWifiProvisioned())
    return ensureSetupMode();

  const std::string readyUrl =
      "http://" + m_deviceIdentity.hostname() + ".local";
  ESP_ERROR_CHECK_WITHOUT_ABORT(m_displayService.showConnecting(readyUrl.c_str()));

  ESP_LOGI(TAG, "Connecting to saved Wi-Fi");
  ESP_RETURN_ON_ERROR(m_wifiManager.connectSaved(m_deviceIdentity.hostname()), TAG,
                      "Failed to connect to saved Wi-Fi");

  if (!m_wifiManager.waitForStaIp(pdMS_TO_TICKS(config::kStaConnectTimeoutMs))) {
    ESP_LOGW(TAG, "Wi-Fi connection timed out, switching to setup mode");
    return ensureSetupMode();
  }

  return ensureReady();
}

esp_err_t ConnectivityCoordinator::ensureSetupMode() {
  const std::string apSsid =
      std::string(config::kSoftApSsidPrefix) + m_deviceIdentity.shortIdUpper();

  ESP_RETURN_ON_ERROR(m_wifiManager.startSetupSoftAp(apSsid), TAG,
                      "Failed to start setup SoftAP");
  ESP_RETURN_ON_ERROR(m_bleProvisioningService.begin(), TAG,
                      "Failed to start BLE provisioning");
  ESP_RETURN_ON_ERROR(m_bleDisplayService.begin(), TAG,
                      "Failed to start BLE display");

  ESP_ERROR_CHECK_WITHOUT_ABORT(m_displayService.showSetupScreen(
      m_deviceIdentity, apSsid.c_str(), config::kSoftApUrl, nullptr,
      m_deviceIdentity.name().c_str()));
  m_setupModeActive = true;
  return ESP_OK;
}

esp_err_t ConnectivityCoordinator::ensureReady() {
  ESP_RETURN_ON_ERROR(m_mdnsService.begin(m_deviceIdentity), TAG,
                      "Failed to start mDNS");
  ESP_RETURN_ON_ERROR(m_mdnsService.addHttpService(), TAG,
                      "Failed to add HTTP service to mDNS");

  const std::string readyUrl =
      "http://" + m_deviceIdentity.hostname() + ".local";
  ESP_RETURN_ON_ERROR(m_displayService.showReady(readyUrl.c_str()), TAG,
                      "Failed to render ready screen");
  m_setupModeActive = false;
  return ESP_OK;
}

void ConnectivityCoordinator::monitorLoop() {
  while (true) {
    const bool provisioned = m_storage.isWifiProvisioned();
    const bool connected = m_wifiManager.hasStaIp();

    if (!provisioned || !connected) {
      if (!m_setupModeActive) {
        ESP_LOGW(TAG, "STA unavailable, enabling setup transports");
        ESP_ERROR_CHECK_WITHOUT_ABORT(ensureSetupMode());
      }
    } else if (m_setupModeActive) {
      ESP_ERROR_CHECK_WITHOUT_ABORT(ensureReady());
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
