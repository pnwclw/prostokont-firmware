#include "Display.h"
#include "esp_log.h"
#include "services/app/ConnectivityCoordinator.hpp"
#include "services/ble/BleDisplayService.hpp"
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
  static BleDisplayService bleDisplay(deviceIdentity, storage, wifiManager,
                                      displayService);
  static HttpServer httpServer(deviceIdentity, storage, wifiManager, bleProvisioning,
                               displayService, mdns);
  static ConnectivityCoordinator connectivityCoordinator(
      deviceIdentity, storage, wifiManager, bleProvisioning, bleDisplay,
      displayService, mdns);
  ESP_ERROR_CHECK(httpServer.begin());
  err = connectivityCoordinator.begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Connectivity coordinator init failed: %s",
             esp_err_to_name(err));
  }
}
