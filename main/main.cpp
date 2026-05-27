#include "Display.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "services/ble/BleProvisioningService.hpp"
#include "services/device/DeviceIdentity.hpp"
#include "services/display/DisplayService.hpp"
#include "services/http/HttpServer.hpp"
#include "services/network/MdnsService.hpp"
#include "services/storage/Storage.hpp"
#include "services/wifi/WifiManager.hpp"

#include "config/AppConfig.hpp"

#include <string>

static const char *TAG = "FRAME_APP";

namespace {

struct ConnectivityMonitorContext {
  DeviceIdentity *deviceIdentity;
  Storage *storage;
  WifiManager *wifiManager;
  BleProvisioningService *bleProvisioning;
};

void connectivityMonitorTask(void *arg) {
  auto *ctx = static_cast<ConnectivityMonitorContext *>(arg);
  bool setupModeStarted = !ctx->storage->isWifiProvisioned();

  while (true) {
    const bool provisioned = ctx->storage->isWifiProvisioned();
    const bool connected = ctx->wifiManager->hasStaIp();

    if (!provisioned || !connected) {
      if (!setupModeStarted) {
        const std::string apSsid =
            std::string(config::kSoftApSsidPrefix) +
            ctx->deviceIdentity->shortIdUpper();
        ESP_LOGW(TAG, "STA unavailable, enabling setup transports");
        ESP_ERROR_CHECK_WITHOUT_ABORT(ctx->wifiManager->startSetupSoftAp(apSsid));
        ESP_ERROR_CHECK_WITHOUT_ABORT(ctx->bleProvisioning->begin());
        setupModeStarted = true;
      }
    } else {
      setupModeStarted = false;
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

} // namespace

extern "C" void app_main(void) {
  static Display display;
  static DisplayService displayService(display);
  ESP_ERROR_CHECK(displayService.begin());

  /*if (config::kShowStartupColorBars) {
    ESP_ERROR_CHECK(displayService.showStartupColorBars());
    vTaskDelay(pdMS_TO_TICKS(config::kStartupColorBarsDurationMs));
  }*/

  static Storage storage;
  esp_err_t err = storage.begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Storage init failed: %s", esp_err_to_name(err));
    //ESP_ERROR_CHECK(displayService.showError("Storage init failed"));
    return;
  }

  static DeviceIdentity deviceIdentity;
  err = deviceIdentity.begin(storage);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Device identity init failed: %s", esp_err_to_name(err));
    //ESP_ERROR_CHECK(displayService.showError("Identity init failed"));
    return;
  }

  static WifiManager wifiManager;
  err = wifiManager.begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "WiFi manager init failed: %s", esp_err_to_name(err));
    //ESP_ERROR_CHECK(displayService.showError("WiFi init failed"));
    return;
  }

  static MdnsService mdns;
  static BleProvisioningService bleProvisioning(deviceIdentity, storage,
                                                wifiManager, displayService,
                                                mdns);
  static HttpServer httpServer(deviceIdentity, storage, wifiManager,
                               bleProvisioning,
                               displayService, mdns);
  ESP_ERROR_CHECK(httpServer.begin());
  static ConnectivityMonitorContext connectivityMonitorContext = {
      .deviceIdentity = &deviceIdentity,
      .storage = &storage,
      .wifiManager = &wifiManager,
      .bleProvisioning = &bleProvisioning,
  };
  static bool connectivityMonitorStarted = false;
  if (!connectivityMonitorStarted) {
    if (xTaskCreate(connectivityMonitorTask, "wifi_setup_monitor", 4096,
                    &connectivityMonitorContext, 4, nullptr) == pdPASS) {
      connectivityMonitorStarted = true;
    }
  }

  if (!storage.isWifiProvisioned()) {
    std::string apSsid =
        std::string(config::kSoftApSsidPrefix) + deviceIdentity.shortIdUpper();
    ESP_ERROR_CHECK(wifiManager.startSetupSoftAp(apSsid));
    ESP_ERROR_CHECK(bleProvisioning.begin());
    //ESP_ERROR_CHECK(displayService.showSetupScreen(deviceIdentity, apSsid.c_str(),
    //                                               config::kSoftApUrl, nullptr));
    return;
  }

  ESP_LOGI(TAG, "Connecting to saved WiFi");
  //ESP_ERROR_CHECK(displayService.showConnecting(deviceIdentity.hostname().c_str()));
  err = wifiManager.connectSaved(deviceIdentity.hostname());
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "WiFi connect failed: %s", esp_err_to_name(err));
    //ESP_ERROR_CHECK(displayService.showError("WiFi connect failed"));
    return;
  }

  if (!wifiManager.waitForStaIp(pdMS_TO_TICKS(config::kStaConnectTimeoutMs))) {
    ESP_LOGW(TAG, "WiFi connection timed out, switching to setup AP");
    std::string apSsid =
        std::string(config::kSoftApSsidPrefix) + deviceIdentity.shortIdUpper();
    ESP_ERROR_CHECK(wifiManager.startSetupSoftAp(apSsid));
    ESP_ERROR_CHECK(bleProvisioning.begin());
    //ESP_ERROR_CHECK(displayService.showSetupScreen(deviceIdentity, apSsid.c_str(),
    //                                               config::kSoftApUrl, nullptr));
    return;
  }

  ESP_ERROR_CHECK(mdns.begin(deviceIdentity));
  ESP_ERROR_CHECK(mdns.addHttpService());
  ESP_ERROR_CHECK(displayService.showReady(deviceIdentity.hostname().c_str()));
}
