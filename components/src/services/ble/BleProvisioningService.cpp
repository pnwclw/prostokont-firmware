/**
 * @file BleProvisioningService.cpp
 * @brief BLE provisioning service for AccessorySetupKit-driven Wi-Fi setup.
 */

#include "services/ble/BleProvisioningService.hpp"

#include "cJSON.h"
#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_store.h"
#include "host/ble_uuid.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/device/DeviceIdentity.hpp"
#include "services/display/DisplayService.hpp"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/network/MdnsService.hpp"
#include "services/storage/Storage.hpp"
#include "services/wifi/WifiManager.hpp"

#include <cstring>
#include <string>

namespace {

extern "C" void ble_store_config_init(void);

constexpr const char *TAG = "ESP_BLE_PROV";
constexpr size_t kMaxDeviceNameBytes = 29;
constexpr size_t kMaxBleJsonBytes = 256;
constexpr size_t kMaxBleStatusBytes = 320;

constexpr ble_uuid128_t kServiceUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x00, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kPairUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x01, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kStatusUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x02, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kConfigUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x03, 0x30, 0x3f, 0x7a);

struct WifiCredentials {
  char ssid[33];
  char password[65];
};

enum class ProvisioningState {
  Idle,
  Connecting,
  Connected,
  Failed,
};

struct Runtime {
  DeviceIdentity *deviceIdentity = nullptr;
  Storage *storage = nullptr;
  WifiManager *wifiManager = nullptr;
  DisplayService *displayService = nullptr;
  MdnsService *mdnsService = nullptr;
  SemaphoreHandle_t mutex = nullptr;
  QueueHandle_t requestQueue = nullptr;
  bool started = false;
  bool notifying = false;
  uint8_t ownAddrType = 0;
  uint16_t connHandle = BLE_HS_CONN_HANDLE_NONE;
  uint16_t pairHandle = 0;
  uint16_t statusHandle = 0;
  uint16_t configHandle = 0;
  ProvisioningState state = ProvisioningState::Idle;
  char lastError[32] = {};
  std::string advertisingName;
};

Runtime g_runtime;

const char *stateName(ProvisioningState state) {
  switch (state) {
  case ProvisioningState::Idle:
    return "idle";
  case ProvisioningState::Connecting:
    return "connecting";
  case ProvisioningState::Connected:
    return "connected";
  case ProvisioningState::Failed:
    return "failed";
  }

  return "unknown";
}

std::string jsonEscape(const char *value) {
  std::string out;
  if (value == nullptr)
    return out;

  for (const char *p = value; *p != '\0'; ++p) {
    switch (*p) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    default:
      out.push_back(*p);
      break;
    }
  }

  return out;
}

void setStateLocked(ProvisioningState state, const char *lastError) {
  g_runtime.state = state;
  std::strncpy(g_runtime.lastError, lastError != nullptr ? lastError : "",
               sizeof(g_runtime.lastError) - 1);
  g_runtime.lastError[sizeof(g_runtime.lastError) - 1] = '\0';
}

std::string statusJsonLocked() {
  const std::string apiBase = "http://" + g_runtime.deviceIdentity->hostname() +
                              ".local" + std::string(config::kApiBasePath);

  std::string json = "{\"state\":\"";
  json += stateName(g_runtime.state);
  json += "\",\"device_id\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->deviceId().c_str());
  json += "\",\"name\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->name().c_str());
  json += "\",\"hostname\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->hostname().c_str());
  json += "\",\"wifi_provisioned\":";
  json += g_runtime.storage->isWifiProvisioned() ? "true" : "false";
  json += ",\"sta_connected\":";
  json += g_runtime.wifiManager->hasStaIp() ? "true" : "false";
  json += ",\"api\":\"";
  json += jsonEscape(apiBase.c_str());
  json += "\"";
  if (g_runtime.lastError[0] != '\0') {
    json += ",\"last_error\":\"";
    json += jsonEscape(g_runtime.lastError);
    json += "\"";
  }
  json += "}";
  return json;
}

std::string pairJsonLocked() {
  std::string json = "{\"protocol\":\"ask-wifi-v1\",\"service_uuid\":";
  json += "\"7A3F3000-43C0-4B45-A47C-D85C67E12411\"";
  json += ",\"device_id\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->deviceId().c_str());
  json += "\",\"name\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->name().c_str());
  json += "\",\"hostname\":\"";
  json += jsonEscape(g_runtime.deviceIdentity->hostname().c_str());
  json += "\"}";
  return json;
}

void notifyStatus();

void updateState(ProvisioningState state, const char *lastError) {
  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
    setStateLocked(state, lastError);
    xSemaphoreGive(g_runtime.mutex);
  }
  notifyStatus();
}

int appendStringToMbuf(struct os_mbuf *om, const std::string &value) {
  return os_mbuf_append(om, value.data(), value.size());
}

int readMbuf(struct os_mbuf *om, char *buffer, size_t capacity, uint16_t *outLen) {
  const uint16_t len = OS_MBUF_PKTLEN(om);
  if (len == 0 || len >= capacity)
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

  int rc = ble_hs_mbuf_to_flat(om, buffer, capacity - 1, outLen);
  if (rc != 0)
    return BLE_ATT_ERR_UNLIKELY;

  buffer[*outLen] = '\0';
  return 0;
}

int characteristicAccess(uint16_t connHandle, uint16_t attrHandle,
                         ble_gatt_access_ctxt *ctxt, void *arg) {
  (void)connHandle;
  (void)arg;

  if (attrHandle == g_runtime.pairHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
      return BLE_ATT_ERR_UNLIKELY;
    const std::string json = pairJsonLocked();
    xSemaphoreGive(g_runtime.mutex);
    return appendStringToMbuf(ctxt->om, json) == 0
               ? 0
               : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attrHandle == g_runtime.statusHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
      return BLE_ATT_ERR_UNLIKELY;
    const std::string json = statusJsonLocked();
    xSemaphoreGive(g_runtime.mutex);
    return appendStringToMbuf(ctxt->om, json) == 0
               ? 0
               : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attrHandle == g_runtime.configHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    char body[kMaxBleJsonBytes] = {};
    uint16_t bodyLen = 0;
    int rc = readMbuf(ctxt->om, body, sizeof(body), &bodyLen);
    if (rc != 0) {
      ESP_LOGW(TAG, "Rejected BLE config write: invalid length");
      updateState(ProvisioningState::Failed, "invalid_length");
      return 0;
    }

    ESP_LOGI(TAG, "Received BLE config write: %u bytes", bodyLen);

    cJSON *json = cJSON_Parse(body);
    if (json == nullptr) {
      ESP_LOGW(TAG, "Rejected BLE config write: invalid JSON");
      updateState(ProvisioningState::Failed, "invalid_json");
      return 0;
    }

    const cJSON *ssidItem = cJSON_GetObjectItemCaseSensitive(json, "ssid");
    const cJSON *passwordItem =
        cJSON_GetObjectItemCaseSensitive(json, "password");
    if (!cJSON_IsString(ssidItem) || ssidItem->valuestring == nullptr ||
        ssidItem->valuestring[0] == '\0') {
      cJSON_Delete(json);
      ESP_LOGW(TAG, "Rejected BLE config write: ssid missing");
      updateState(ProvisioningState::Failed, "ssid_required");
      return 0;
    }

    WifiCredentials creds = {};
    std::strncpy(creds.ssid, ssidItem->valuestring, sizeof(creds.ssid) - 1);
    if (cJSON_IsString(passwordItem) && passwordItem->valuestring != nullptr) {
      std::strncpy(creds.password, passwordItem->valuestring,
                   sizeof(creds.password) - 1);
    }
    cJSON_Delete(json);

    if (xQueueOverwrite(g_runtime.requestQueue, &creds) != pdPASS) {
      ESP_LOGW(TAG, "Rejected BLE config write: queue failed");
      updateState(ProvisioningState::Failed, "queue_failed");
      return 0;
    }

    ESP_LOGI(TAG, "Accepted BLE config write for SSID '%s'", creds.ssid);
    updateState(ProvisioningState::Connecting, "");
    return 0;
  }

  return BLE_ATT_ERR_UNLIKELY;
}

ble_gatt_chr_def g_characteristics[] = {
    {
        .uuid = &kPairUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
        .min_key_size = 0,
        .val_handle = &g_runtime.pairHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kStatusUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY |
                 BLE_GATT_CHR_F_READ_ENC,
        .min_key_size = 0,
        .val_handle = &g_runtime.statusHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kConfigUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
        .min_key_size = 0,
        .val_handle = &g_runtime.configHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = nullptr,
        .access_cb = nullptr,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = 0,
        .min_key_size = 0,
        .val_handle = nullptr,
        .cpfd = nullptr,
    },
};

const ble_gatt_svc_def g_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &kServiceUuid.u,
        .includes = nullptr,
        .characteristics = g_characteristics,
    },
    {
        .type = 0,
        .uuid = nullptr,
        .includes = nullptr,
        .characteristics = nullptr,
    },
};

void advertise();

int gapEvent(ble_gap_event *event, void *arg) {
  (void)arg;

  switch (event->type) {
  case BLE_GAP_EVENT_CONNECT:
    if (event->connect.status == 0) {
      g_runtime.connHandle = event->connect.conn_handle;
      ESP_LOGI(TAG, "BLE connected, conn_handle=%u", g_runtime.connHandle);
    } else {
      ESP_LOGW(TAG, "BLE connect failed: %d", event->connect.status);
      advertise();
    }
    return 0;
  case BLE_GAP_EVENT_DISCONNECT:
    ESP_LOGI(TAG, "BLE disconnected, reason=%d", event->disconnect.reason);
    g_runtime.connHandle = BLE_HS_CONN_HANDLE_NONE;
    g_runtime.notifying = false;
    advertise();
    return 0;
  case BLE_GAP_EVENT_ADV_COMPLETE:
    ESP_LOGI(TAG, "BLE advertising completed, restarting");
    advertise();
    return 0;
  case BLE_GAP_EVENT_SUBSCRIBE:
    if (event->subscribe.attr_handle == g_runtime.statusHandle) {
      g_runtime.notifying = event->subscribe.cur_notify != 0;
      ESP_LOGI(TAG, "BLE status notifications %s",
               g_runtime.notifying ? "enabled" : "disabled");
      notifyStatus();
    }
    return 0;
  case BLE_GAP_EVENT_REPEAT_PAIRING: {
    ble_gap_conn_desc desc = {};
    if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0) {
      ble_store_util_delete_peer(&desc.peer_id_addr);
    }
    return BLE_GAP_REPEAT_PAIRING_RETRY;
  }
  default:
    return 0;
  }
}

void advertise() {
  ble_hs_adv_fields fields = {};
  ble_uuid128_t serviceUuid = kServiceUuid;
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.uuids128 = &serviceUuid;
  fields.num_uuids128 = 1;
  fields.uuids128_is_complete = 1;

  int rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set BLE advertisement fields: %d", rc);
    return;
  }

  ble_hs_adv_fields scanResponse = {};
  scanResponse.name =
      reinterpret_cast<uint8_t *>(g_runtime.advertisingName.data());
  scanResponse.name_len = g_runtime.advertisingName.size();
  scanResponse.name_is_complete = 1;
  rc = ble_gap_adv_rsp_set_fields(&scanResponse);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set BLE scan response fields: %d", rc);
    return;
  }

  ble_gap_adv_params params = {};
  params.conn_mode = BLE_GAP_CONN_MODE_UND;
  params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  ESP_LOGI(TAG, "Starting BLE advertising: name='%s'",
           g_runtime.advertisingName.c_str());
  rc = ble_gap_adv_start(g_runtime.ownAddrType, nullptr, BLE_HS_FOREVER, &params,
                         gapEvent, nullptr);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to start BLE advertisement: %d", rc);
  } else {
    ESP_LOGI(TAG, "BLE advertising started");
  }
}

void onSync() {
  int rc = ble_hs_id_infer_auto(0, &g_runtime.ownAddrType);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: %d", rc);
    return;
  }
  ESP_LOGI(TAG, "NimBLE host synced, own_addr_type=%u", g_runtime.ownAddrType);
  advertise();
}

void onReset(int reason) { ESP_LOGW(TAG, "NimBLE reset: %d", reason); }

void hostTask(void *param) {
  (void)param;
  ESP_LOGI(TAG, "NimBLE host task started");
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void workerTask(void *param) {
  (void)param;

  WifiCredentials creds = {};
  while (true) {
    if (xQueueReceive(g_runtime.requestQueue, &creds, portMAX_DELAY) != pdPASS)
      continue;

    ESP_LOGI(TAG, "Applying BLE Wi-Fi credentials for SSID '%s'", creds.ssid);
    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showConnecting(creds.ssid));
    updateState(ProvisioningState::Connecting, "");

    esp_err_t err = g_runtime.wifiManager->configureAndConnect(
        creds.ssid, creds.password, g_runtime.deviceIdentity->hostname());
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Wi-Fi config failed: %s", esp_err_to_name(err));
      updateState(ProvisioningState::Failed, "wifi_config_failed");
      ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showError("WiFi config failed"));
      continue;
    }

    if (!g_runtime.wifiManager->waitForStaIp(
            pdMS_TO_TICKS(config::kStaConnectTimeoutMs))) {
      ESP_LOGW(TAG, "Wi-Fi connect timeout after BLE provisioning");
      updateState(ProvisioningState::Failed, "connect_timeout");
      ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showError("WiFi timeout"));
      continue;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.storage->setWifiProvisioned(true));

    err = g_runtime.mdnsService->begin(*g_runtime.deviceIdentity);
    if (err == ESP_OK)
      err = g_runtime.mdnsService->addHttpService();
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "mDNS init failed after BLE provisioning: %s",
               esp_err_to_name(err));
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showReady(
        g_runtime.deviceIdentity->hostname().c_str()));
    updateState(ProvisioningState::Connected, err == ESP_OK ? "" : "mdns_failed");
  }
}

void notifyStatus() {
  if (!g_runtime.notifying || g_runtime.connHandle == BLE_HS_CONN_HANDLE_NONE ||
      g_runtime.statusHandle == 0) {
    return;
  }

  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
    return;
  std::string json = statusJsonLocked();
  xSemaphoreGive(g_runtime.mutex);

  if (json.size() > kMaxBleStatusBytes)
    json.resize(kMaxBleStatusBytes);

  os_mbuf *om = ble_hs_mbuf_from_flat(json.data(), json.size());
  if (om == nullptr) {
    ESP_LOGW(TAG, "Failed to allocate BLE notify buffer");
    return;
  }

  int rc = ble_gatts_notify_custom(g_runtime.connHandle, g_runtime.statusHandle, om);
  if (rc != 0) {
    ESP_LOGW(TAG, "Failed to notify BLE status: %d", rc);
  }
}

} // namespace

BleProvisioningService::BleProvisioningService(
    DeviceIdentity &deviceIdentity, Storage &storage, WifiManager &wifiManager,
    DisplayService &displayService, MdnsService &mdnsService)
    : m_deviceIdentity(deviceIdentity), m_storage(storage),
      m_wifiManager(wifiManager), m_displayService(displayService),
      m_mdnsService(mdnsService) {}

esp_err_t BleProvisioningService::begin() {
  if (g_runtime.started)
    return ESP_OK;

  g_runtime.deviceIdentity = &m_deviceIdentity;
  g_runtime.storage = &m_storage;
  g_runtime.wifiManager = &m_wifiManager;
  g_runtime.displayService = &m_displayService;
  g_runtime.mdnsService = &m_mdnsService;
  g_runtime.advertisingName = m_deviceIdentity.name().substr(0, kMaxDeviceNameBytes);
  g_runtime.mutex = xSemaphoreCreateMutex();
  g_runtime.requestQueue = xQueueCreate(1, sizeof(WifiCredentials));
  if (g_runtime.mutex == nullptr || g_runtime.requestQueue == nullptr)
    return ESP_ERR_NO_MEM;

  ESP_LOGI(TAG, "Initializing BLE provisioning");
  updateState(ProvisioningState::Idle, "");

  ESP_RETURN_ON_ERROR(nimble_port_init(), TAG, "Failed to initialize NimBLE");

  ble_hs_cfg.reset_cb = onReset;
  ble_hs_cfg.sync_cb = onSync;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
  ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
  ble_hs_cfg.sm_bonding = 1;
  ble_hs_cfg.sm_sc = 1;
  ble_hs_cfg.sm_mitm = 0;
  ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
  ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  int rc = ble_svc_gap_device_name_set(g_runtime.advertisingName.c_str());
  if (rc != 0)
    return ESP_FAIL;

  rc = ble_gatts_count_cfg(g_services);
  if (rc != 0)
    return ESP_FAIL;

  rc = ble_gatts_add_svcs(g_services);
  if (rc != 0)
    return ESP_FAIL;

  ble_store_config_init();
  nimble_port_freertos_init(hostTask);

  if (xTaskCreate(workerTask, "ble_prov_worker", 6144, nullptr, 5, nullptr) !=
      pdPASS) {
    return ESP_ERR_NO_MEM;
  }

  g_runtime.started = true;
  ESP_LOGI(TAG, "BLE provisioning started with name '%s'",
           g_runtime.advertisingName.c_str());
  return ESP_OK;
}
