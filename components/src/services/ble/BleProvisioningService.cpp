/**
 * @file BleProvisioningService.cpp
 * @brief BLE provisioning and display services backed by one NimBLE host.
 */

#include "services/ble/BleProvisioningService.hpp"

#include "services/ble/BleDisplayService.hpp"

#include "cJSON.h"
#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_heap_caps.h"
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
#include "services/board/BoardProfile.hpp"
#include "services/device/DeviceIdentity.hpp"
#include "services/device/StatusPayloadJson.hpp"
#include "services/display/DisplayService.hpp"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/network/MdnsService.hpp"
#include "services/storage/Storage.hpp"
#include "services/wifi/WifiManager.hpp"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

extern "C" void ble_store_config_init(void);

constexpr const char *TAG = "ESP_BLE_STACK";
constexpr size_t kMaxDeviceNameBytes = 29;
constexpr size_t kMaxBleJsonBytes = 256;
constexpr size_t kMaxBleStatusBytes = 2048;
constexpr size_t kMaxFormatBytes = 8;

constexpr ble_uuid128_t kProvisioningServiceUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x00, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kProvisioningPairUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x01, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kProvisioningStatusUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x02, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kProvisioningConfigUuid =
    BLE_UUID128_INIT(0x11, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x03, 0x30, 0x3f, 0x7a);

constexpr ble_uuid128_t kDisplayServiceUuid =
    BLE_UUID128_INIT(0x22, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x00, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kDisplayStatusUuid =
    BLE_UUID128_INIT(0x22, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x01, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kDisplayControlUuid =
    BLE_UUID128_INIT(0x22, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x02, 0x30, 0x3f, 0x7a);
constexpr ble_uuid128_t kDisplayDataUuid =
    BLE_UUID128_INIT(0x22, 0x24, 0xe1, 0x67, 0x5c, 0xd8, 0x7c, 0xa4, 0x45, 0x4b,
                     0xc0, 0x43, 0x03, 0x30, 0x3f, 0x7a);
constexpr const char *kProvisioningServiceUuidText =
    "7A3F3000-43C0-4B45-A47C-D85C67E12411";

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

enum class DisplayState {
  Idle,
  Receiving,
  Rendering,
  Ready,
  Failed,
};

enum class DisplayCommandType {
  Clear,
  Commit,
};

struct DisplayCommand {
  DisplayCommandType type;
};

struct UploadSession {
  bool active = false;
  bool renderPending = false;
  bool dither = true;
  char format[kMaxFormatBytes] = {};
  size_t expectedSize = 0;
  size_t receivedSize = 0;
  size_t capacity = 0;
  uint8_t *buffer = nullptr;
};

struct Runtime {
  DeviceIdentity *deviceIdentity = nullptr;
  Storage *storage = nullptr;
  WifiManager *wifiManager = nullptr;
  DisplayService *displayService = nullptr;
  MdnsService *mdnsService = nullptr;
  SemaphoreHandle_t mutex = nullptr;
  QueueHandle_t provisioningQueue = nullptr;
  QueueHandle_t displayQueue = nullptr;
  bool runtimeReady = false;
  bool hostStarted = false;
  bool provisioningNotifyEnabled = false;
  bool displayNotifyEnabled = false;
  uint8_t ownAddrType = 0;
  uint16_t connHandle = BLE_HS_CONN_HANDLE_NONE;
  uint16_t provisioningPairHandle = 0;
  uint16_t provisioningStatusHandle = 0;
  uint16_t provisioningConfigHandle = 0;
  uint16_t displayStatusHandle = 0;
  uint16_t displayControlHandle = 0;
  uint16_t displayDataHandle = 0;
  ProvisioningState provisioningState = ProvisioningState::Idle;
  DisplayState displayState = DisplayState::Idle;
  char provisioningError[32] = {};
  char displayError[32] = {};
  std::string advertisingName;
  UploadSession upload;
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

const char *stateName(DisplayState state) {
  switch (state) {
  case DisplayState::Idle:
    return "idle";
  case DisplayState::Receiving:
    return "receiving";
  case DisplayState::Rendering:
    return "rendering";
  case DisplayState::Ready:
    return "ready";
  case DisplayState::Failed:
    return "failed";
  }

  return "unknown";
}

void setProvisioningStateLocked(ProvisioningState state, const char *lastError) {
  g_runtime.provisioningState = state;
  std::strncpy(g_runtime.provisioningError, lastError != nullptr ? lastError : "",
               sizeof(g_runtime.provisioningError) - 1);
  g_runtime.provisioningError[sizeof(g_runtime.provisioningError) - 1] = '\0';
}

void setDisplayStateLocked(DisplayState state, const char *lastError) {
  g_runtime.displayState = state;
  std::strncpy(g_runtime.displayError, lastError != nullptr ? lastError : "",
               sizeof(g_runtime.displayError) - 1);
  g_runtime.displayError[sizeof(g_runtime.displayError) - 1] = '\0';
}

void freeUploadBufferLocked() {
  if (g_runtime.upload.buffer != nullptr) {
    heap_caps_free(g_runtime.upload.buffer);
    g_runtime.upload.buffer = nullptr;
  }
  g_runtime.upload = UploadSession{};
}

std::string provisioningStatusJsonLocked() {
  const BoardProfile &board = currentBoardProfile();
  cJSON *json = cJSON_CreateObject();
  prostokont::status_json::addState(json,
                                    stateName(g_runtime.provisioningState));
  prostokont::status_json::addRuntimeStatus(
      json, *g_runtime.deviceIdentity, board,
      g_runtime.storage->isWifiProvisioned(), g_runtime.wifiManager->hasStaIp());
  prostokont::status_json::addApiBase(json, *g_runtime.deviceIdentity,
                                      config::kApiBasePath);
  prostokont::status_json::addLastError(json, g_runtime.provisioningError);

  std::string rendered = prostokont::status_json::printUnformatted(json);
  cJSON_Delete(json);
  return rendered;
}

std::string displayStatusJsonLocked() {
  const BoardProfile &board = currentBoardProfile();
  cJSON *json = cJSON_CreateObject();
  prostokont::status_json::addState(json, stateName(g_runtime.displayState));
  prostokont::status_json::addRuntimeStatus(
      json, *g_runtime.deviceIdentity, board,
      g_runtime.storage->isWifiProvisioned(), g_runtime.wifiManager->hasStaIp());
  prostokont::status_json::addDisplayTransferStatus(
      json, "ble-display-v1", g_runtime.upload.expectedSize,
      g_runtime.upload.receivedSize, g_runtime.upload.format);
  prostokont::status_json::addLastError(json, g_runtime.displayError);

  std::string rendered = prostokont::status_json::printUnformatted(json);
  cJSON_Delete(json);
  return rendered;
}

std::string pairJsonLocked() {
  cJSON *json = cJSON_CreateObject();
  prostokont::status_json::addString(json, "protocol", "ask-wifi-v1");
  prostokont::status_json::addString(json, "service_uuid",
                                     kProvisioningServiceUuidText);
  prostokont::status_json::addDeviceIdentity(json, *g_runtime.deviceIdentity);

  std::string rendered = prostokont::status_json::printUnformatted(json);
  cJSON_Delete(json);
  return rendered;
}

void notifyProvisioningStatus();
void notifyDisplayStatus();
void advertise();

void updateProvisioningState(ProvisioningState state, const char *lastError) {
  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
    setProvisioningStateLocked(state, lastError);
    xSemaphoreGive(g_runtime.mutex);
  }
  notifyProvisioningStatus();
}

void updateDisplayState(DisplayState state, const char *lastError) {
  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
    setDisplayStateLocked(state, lastError);
    xSemaphoreGive(g_runtime.mutex);
  }
  notifyDisplayStatus();
}

int appendStringToMbuf(struct os_mbuf *om, const std::string &value) {
  return os_mbuf_append(om, value.data(), value.size());
}

int readMbuf(struct os_mbuf *om, char *buffer, size_t capacity,
             uint16_t *outLen) {
  const uint16_t len = OS_MBUF_PKTLEN(om);
  if (len == 0 || len >= capacity)
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

  int rc = ble_hs_mbuf_to_flat(om, buffer, capacity - 1, outLen);
  if (rc != 0)
    return BLE_ATT_ERR_UNLIKELY;

  buffer[*outLen] = '\0';
  return 0;
}

bool isAcceptedDisplayFormat(const char *format) {
  return std::strcmp(format, "jpeg") == 0 || std::strcmp(format, "png") == 0 ||
         std::strcmp(format, "bmp") == 0 || std::strcmp(format, "packed") == 0;
}

void setDisplayUploadLocked(const char *format, size_t expectedSize, bool dither,
                            uint8_t *buffer) {
  freeUploadBufferLocked();
  g_runtime.upload.active = true;
  g_runtime.upload.renderPending = false;
  g_runtime.upload.dither = dither;
  g_runtime.upload.expectedSize = expectedSize;
  g_runtime.upload.capacity = expectedSize;
  g_runtime.upload.receivedSize = 0;
  g_runtime.upload.buffer = buffer;
  std::strncpy(g_runtime.upload.format, format, sizeof(g_runtime.upload.format) - 1);
  g_runtime.upload.format[sizeof(g_runtime.upload.format) - 1] = '\0';
}

esp_err_t queueDisplayCommand(DisplayCommandType type) {
  const DisplayCommand command{type};
  if (xQueueSend(g_runtime.displayQueue, &command, 0) != pdPASS)
    return ESP_ERR_INVALID_STATE;

  return ESP_OK;
}

esp_err_t handleDisplayControlWrite(const char *body) {
  cJSON *json = cJSON_Parse(body);
  if (json == nullptr) {
    updateDisplayState(DisplayState::Failed, "invalid_json");
    return ESP_ERR_INVALID_ARG;
  }

  const cJSON *opItem = cJSON_GetObjectItemCaseSensitive(json, "op");
  if (!cJSON_IsString(opItem) || opItem->valuestring == nullptr) {
    cJSON_Delete(json);
    updateDisplayState(DisplayState::Failed, "op_required");
    return ESP_ERR_INVALID_ARG;
  }

  const char *op = opItem->valuestring;
  if (std::strcmp(op, "clear") == 0) {
    cJSON_Delete(json);
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
      freeUploadBufferLocked();
      setDisplayStateLocked(DisplayState::Rendering, "");
      xSemaphoreGive(g_runtime.mutex);
    }
    ESP_RETURN_ON_ERROR(queueDisplayCommand(DisplayCommandType::Clear), TAG,
                        "Failed to queue display clear");
    notifyDisplayStatus();
    return ESP_OK;
  }

  if (std::strcmp(op, "abort") == 0) {
    cJSON_Delete(json);
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
      freeUploadBufferLocked();
      setDisplayStateLocked(DisplayState::Idle, "");
      xSemaphoreGive(g_runtime.mutex);
    }
    notifyDisplayStatus();
    return ESP_OK;
  }

  if (std::strcmp(op, "begin") == 0) {
    const cJSON *formatItem = cJSON_GetObjectItemCaseSensitive(json, "format");
    const cJSON *sizeItem = cJSON_GetObjectItemCaseSensitive(json, "size");
    const cJSON *ditherItem = cJSON_GetObjectItemCaseSensitive(json, "dither");
    if (!cJSON_IsString(formatItem) || formatItem->valuestring == nullptr ||
        !isAcceptedDisplayFormat(formatItem->valuestring) ||
        !cJSON_IsNumber(sizeItem) || sizeItem->valuedouble <= 0) {
      cJSON_Delete(json);
      updateDisplayState(DisplayState::Failed, "invalid_begin");
      return ESP_ERR_INVALID_ARG;
    }

    const size_t expectedSize = static_cast<size_t>(sizeItem->valuedouble);
    if (expectedSize > config::kMaxImageUploadBytes) {
      cJSON_Delete(json);
      updateDisplayState(DisplayState::Failed, "image_too_large");
      return ESP_ERR_INVALID_SIZE;
    }
    if (std::strcmp(formatItem->valuestring, "packed") == 0 &&
        (!supportsDisplayInputFormat(currentBoardProfile(),
                                     kDisplayInputFormatPackedFrame) ||
         expectedSize != currentPackedFrameBytes())) {
      cJSON_Delete(json);
      updateDisplayState(DisplayState::Failed, "invalid_packed_size");
      return ESP_ERR_INVALID_SIZE;
    }

    const bool dither = !cJSON_IsBool(ditherItem) || cJSON_IsTrue(ditherItem);
    uint8_t *buffer = static_cast<uint8_t *>(heap_caps_malloc(
        expectedSize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM));
    if (buffer == nullptr) {
      buffer = static_cast<uint8_t *>(heap_caps_malloc(expectedSize, MALLOC_CAP_8BIT));
    }
    if (buffer == nullptr) {
      cJSON_Delete(json);
      updateDisplayState(DisplayState::Failed, "out_of_memory");
      return ESP_ERR_NO_MEM;
    }

    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
      setDisplayUploadLocked(formatItem->valuestring, expectedSize, dither, buffer);
      setDisplayStateLocked(DisplayState::Receiving, "");
      xSemaphoreGive(g_runtime.mutex);
    }
    cJSON_Delete(json);
    notifyDisplayStatus();
    return ESP_OK;
  }

  if (std::strcmp(op, "commit") == 0) {
    cJSON_Delete(json);
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE) {
      updateDisplayState(DisplayState::Failed, "mutex_failed");
      return ESP_FAIL;
    }

    const bool readyToCommit =
        g_runtime.upload.active && !g_runtime.upload.renderPending &&
        g_runtime.upload.buffer != nullptr &&
        g_runtime.upload.receivedSize == g_runtime.upload.expectedSize;
    if (!readyToCommit) {
      setDisplayStateLocked(DisplayState::Failed, "upload_incomplete");
      xSemaphoreGive(g_runtime.mutex);
      notifyDisplayStatus();
      return ESP_ERR_INVALID_STATE;
    }

    g_runtime.upload.active = false;
    g_runtime.upload.renderPending = true;
    setDisplayStateLocked(DisplayState::Rendering, "");
    xSemaphoreGive(g_runtime.mutex);

    ESP_RETURN_ON_ERROR(queueDisplayCommand(DisplayCommandType::Commit), TAG,
                        "Failed to queue display render");
    notifyDisplayStatus();
    return ESP_OK;
  }

  cJSON_Delete(json);
  updateDisplayState(DisplayState::Failed, "unsupported_op");
  return ESP_ERR_NOT_SUPPORTED;
}

int characteristicAccess(uint16_t connHandle, uint16_t attrHandle,
                         ble_gatt_access_ctxt *ctxt, void *arg) {
  (void)connHandle;
  (void)arg;

  if (attrHandle == g_runtime.provisioningPairHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
      return BLE_ATT_ERR_UNLIKELY;
    const std::string json = pairJsonLocked();
    xSemaphoreGive(g_runtime.mutex);
    return appendStringToMbuf(ctxt->om, json) == 0
               ? 0
               : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attrHandle == g_runtime.provisioningStatusHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
      return BLE_ATT_ERR_UNLIKELY;
    const std::string json = provisioningStatusJsonLocked();
    xSemaphoreGive(g_runtime.mutex);
    return appendStringToMbuf(ctxt->om, json) == 0
               ? 0
               : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attrHandle == g_runtime.provisioningConfigHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    char body[kMaxBleJsonBytes] = {};
    uint16_t bodyLen = 0;
    int rc = readMbuf(ctxt->om, body, sizeof(body), &bodyLen);
    if (rc != 0) {
      ESP_LOGW(TAG, "Rejected BLE provisioning write: invalid length");
      updateProvisioningState(ProvisioningState::Failed, "invalid_length");
      return 0;
    }

    cJSON *json = cJSON_Parse(body);
    if (json == nullptr) {
      updateProvisioningState(ProvisioningState::Failed, "invalid_json");
      return 0;
    }

    const cJSON *ssidItem = cJSON_GetObjectItemCaseSensitive(json, "ssid");
    const cJSON *passwordItem =
        cJSON_GetObjectItemCaseSensitive(json, "password");
    if (!cJSON_IsString(ssidItem) || ssidItem->valuestring == nullptr ||
        ssidItem->valuestring[0] == '\0') {
      cJSON_Delete(json);
      updateProvisioningState(ProvisioningState::Failed, "ssid_required");
      return 0;
    }

    WifiCredentials creds = {};
    std::strncpy(creds.ssid, ssidItem->valuestring, sizeof(creds.ssid) - 1);
    if (cJSON_IsString(passwordItem) && passwordItem->valuestring != nullptr) {
      std::strncpy(creds.password, passwordItem->valuestring,
                   sizeof(creds.password) - 1);
    }
    cJSON_Delete(json);

    if (xQueueOverwrite(g_runtime.provisioningQueue, &creds) != pdPASS) {
      updateProvisioningState(ProvisioningState::Failed, "queue_failed");
      return 0;
    }

    updateProvisioningState(ProvisioningState::Connecting, "");
    return 0;
  }

  if (attrHandle == g_runtime.displayStatusHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
      return BLE_ATT_ERR_UNLIKELY;
    const std::string json = displayStatusJsonLocked();
    xSemaphoreGive(g_runtime.mutex);
    return appendStringToMbuf(ctxt->om, json) == 0
               ? 0
               : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attrHandle == g_runtime.displayControlHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    char body[kMaxBleJsonBytes] = {};
    uint16_t bodyLen = 0;
    int rc = readMbuf(ctxt->om, body, sizeof(body), &bodyLen);
    if (rc != 0) {
      updateDisplayState(DisplayState::Failed, "invalid_length");
      return 0;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(handleDisplayControlWrite(body));
    return 0;
  }

  if (attrHandle == g_runtime.displayDataHandle &&
      ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    const uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
    if (len == 0)
      return 0;

    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE) {
      updateDisplayState(DisplayState::Failed, "mutex_failed");
      return 0;
    }

    const bool canWrite = g_runtime.upload.active && !g_runtime.upload.renderPending &&
                          g_runtime.upload.buffer != nullptr &&
                          g_runtime.upload.receivedSize + len <=
                              g_runtime.upload.capacity;
    if (!canWrite) {
      setDisplayStateLocked(DisplayState::Failed, "invalid_upload_state");
      xSemaphoreGive(g_runtime.mutex);
      notifyDisplayStatus();
      return 0;
    }

    uint16_t written = 0;
    int rc = ble_hs_mbuf_to_flat(ctxt->om,
                                 g_runtime.upload.buffer + g_runtime.upload.receivedSize,
                                 len, &written);
    if (rc != 0 || written != len) {
      setDisplayStateLocked(DisplayState::Failed, "chunk_copy_failed");
      xSemaphoreGive(g_runtime.mutex);
      notifyDisplayStatus();
      return 0;
    }

    g_runtime.upload.receivedSize += written;
    xSemaphoreGive(g_runtime.mutex);
    notifyDisplayStatus();
    return 0;
  }

  return BLE_ATT_ERR_UNLIKELY;
}

ble_gatt_chr_def g_provisioningCharacteristics[] = {
    {
        .uuid = &kProvisioningPairUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ,
        .min_key_size = 0,
        .val_handle = &g_runtime.provisioningPairHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kProvisioningStatusUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .min_key_size = 0,
        .val_handle = &g_runtime.provisioningStatusHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kProvisioningConfigUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_WRITE,
        .min_key_size = 0,
        .val_handle = &g_runtime.provisioningConfigHandle,
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

ble_gatt_chr_def g_displayCharacteristics[] = {
    {
        .uuid = &kDisplayStatusUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .min_key_size = 0,
        .val_handle = &g_runtime.displayStatusHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kDisplayControlUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_WRITE,
        .min_key_size = 0,
        .val_handle = &g_runtime.displayControlHandle,
        .cpfd = nullptr,
    },
    {
        .uuid = &kDisplayDataUuid.u,
        .access_cb = characteristicAccess,
        .arg = nullptr,
        .descriptors = nullptr,
        .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
        .min_key_size = 0,
        .val_handle = &g_runtime.displayDataHandle,
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
        .uuid = &kProvisioningServiceUuid.u,
        .includes = nullptr,
        .characteristics = g_provisioningCharacteristics,
    },
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &kDisplayServiceUuid.u,
        .includes = nullptr,
        .characteristics = g_displayCharacteristics,
    },
    {
        .type = 0,
        .uuid = nullptr,
        .includes = nullptr,
        .characteristics = nullptr,
    },
};

void notifyHandle(bool enabled, uint16_t connHandle, uint16_t valueHandle,
                  const std::string &payload) {
  if (!enabled || connHandle == BLE_HS_CONN_HANDLE_NONE || valueHandle == 0)
    return;

  os_mbuf *om = ble_hs_mbuf_from_flat(payload.data(), payload.size());
  if (om == nullptr)
    return;

  int rc = ble_gatts_notify_custom(connHandle, valueHandle, om);
  if (rc != 0) {
    ESP_LOGW(TAG, "BLE notify failed for handle %u: %d", valueHandle, rc);
  }
}

int gapEvent(ble_gap_event *event, void *arg) {
  (void)arg;

  switch (event->type) {
  case BLE_GAP_EVENT_CONNECT:
    if (event->connect.status == 0) {
      g_runtime.connHandle = event->connect.conn_handle;
      ESP_LOGI(TAG, "BLE connected, conn_handle=%u", g_runtime.connHandle);
    } else {
      advertise();
    }
    return 0;
  case BLE_GAP_EVENT_DISCONNECT:
    g_runtime.connHandle = BLE_HS_CONN_HANDLE_NONE;
    g_runtime.provisioningNotifyEnabled = false;
    g_runtime.displayNotifyEnabled = false;
    advertise();
    return 0;
  case BLE_GAP_EVENT_ADV_COMPLETE:
    advertise();
    return 0;
  case BLE_GAP_EVENT_SUBSCRIBE:
    if (event->subscribe.attr_handle == g_runtime.provisioningStatusHandle) {
      g_runtime.provisioningNotifyEnabled = event->subscribe.cur_notify != 0;
      notifyProvisioningStatus();
    } else if (event->subscribe.attr_handle == g_runtime.displayStatusHandle) {
      g_runtime.displayNotifyEnabled = event->subscribe.cur_notify != 0;
      notifyDisplayStatus();
    }
    return 0;
  case BLE_GAP_EVENT_REPEAT_PAIRING: {
    ble_gap_conn_desc desc = {};
    if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0)
      ble_store_util_delete_peer(&desc.peer_id_addr);
    return BLE_GAP_REPEAT_PAIRING_RETRY;
  }
  default:
    return 0;
  }
}

void advertise() {
  if (g_runtime.connHandle != BLE_HS_CONN_HANDLE_NONE || ble_gap_adv_active())
    return;

  ble_hs_adv_fields fields = {};
  ble_uuid128_t serviceUuids[2] = {kProvisioningServiceUuid, kDisplayServiceUuid};
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.uuids128 = serviceUuids;
  fields.num_uuids128 = 2;
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

  rc = ble_gap_adv_start(g_runtime.ownAddrType, nullptr, BLE_HS_FOREVER, &params,
                         gapEvent, nullptr);
  if (rc != 0 && rc != BLE_HS_EALREADY)
    ESP_LOGE(TAG, "Failed to start BLE advertising: %d", rc);
}

void onSync() {
  int rc = ble_hs_id_infer_auto(0, &g_runtime.ownAddrType);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: %d", rc);
    return;
  }
  advertise();
}

void onReset(int reason) { ESP_LOGW(TAG, "NimBLE reset: %d", reason); }

void hostTask(void *param) {
  (void)param;
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void provisioningWorkerTask(void *param) {
  (void)param;

  WifiCredentials creds = {};
  while (true) {
    if (xQueueReceive(g_runtime.provisioningQueue, &creds, portMAX_DELAY) != pdPASS)
      continue;

    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showConnecting(creds.ssid));
    updateProvisioningState(ProvisioningState::Connecting, "");

    esp_err_t err = g_runtime.wifiManager->configureAndConnect(
        creds.ssid, creds.password, g_runtime.deviceIdentity->hostname());
    if (err != ESP_OK) {
      updateProvisioningState(ProvisioningState::Failed, "wifi_config_failed");
      ESP_ERROR_CHECK_WITHOUT_ABORT(
          g_runtime.displayService->showError("WiFi config failed"));
      continue;
    }

    if (!g_runtime.wifiManager->waitForStaIp(
            pdMS_TO_TICKS(config::kStaConnectTimeoutMs))) {
      updateProvisioningState(ProvisioningState::Failed, "connect_timeout");
      ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showError("WiFi timeout"));
      continue;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.storage->setWifiProvisioned(true));

    err = g_runtime.mdnsService->begin(*g_runtime.deviceIdentity);
    if (err == ESP_OK)
      err = g_runtime.mdnsService->addHttpService();

    const std::string readyUrl =
        "http://" + g_runtime.deviceIdentity->hostname() + ".local";
    ESP_ERROR_CHECK_WITHOUT_ABORT(g_runtime.displayService->showReady(readyUrl.c_str()));
    updateProvisioningState(ProvisioningState::Connected,
                            err == ESP_OK ? "" : "mdns_failed");
  }
}

esp_err_t renderUploadedFrame(const UploadSession &upload) {
  if (std::strcmp(upload.format, "packed") == 0)
    return g_runtime.displayService->showPackedFrame(upload.buffer, upload.receivedSize);

  return g_runtime.displayService->showImage(upload.buffer, upload.receivedSize,
                                             upload.dither);
}

void displayWorkerTask(void *param) {
  (void)param;

  DisplayCommand command = {};
  while (true) {
    if (xQueueReceive(g_runtime.displayQueue, &command, portMAX_DELAY) != pdPASS)
      continue;

    if (command.type == DisplayCommandType::Clear) {
      esp_err_t err = g_runtime.displayService->clear();
      updateDisplayState(err == ESP_OK ? DisplayState::Idle : DisplayState::Failed,
                         err == ESP_OK ? "" : "display_clear_failed");
      continue;
    }

    UploadSession upload = {};
    if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) == pdTRUE) {
      upload = g_runtime.upload;
      g_runtime.upload.buffer = nullptr;
      g_runtime.upload.capacity = 0;
      g_runtime.upload.expectedSize = 0;
      g_runtime.upload.receivedSize = 0;
      g_runtime.upload.active = false;
      g_runtime.upload.renderPending = false;
      g_runtime.upload.format[0] = '\0';
      xSemaphoreGive(g_runtime.mutex);
    }

    if (upload.buffer == nullptr) {
      updateDisplayState(DisplayState::Failed, "missing_upload");
      continue;
    }

    esp_err_t err = renderUploadedFrame(upload);
    heap_caps_free(upload.buffer);
    updateDisplayState(err == ESP_OK ? DisplayState::Ready : DisplayState::Failed,
                       err == ESP_OK ? "" : "image_render_failed");
  }
}

void notifyProvisioningStatus() {
  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
    return;
  const std::string json = provisioningStatusJsonLocked();
  const bool notify = g_runtime.provisioningNotifyEnabled;
  const uint16_t connHandle = g_runtime.connHandle;
  const uint16_t handle = g_runtime.provisioningStatusHandle;
  xSemaphoreGive(g_runtime.mutex);
  notifyHandle(notify, connHandle, handle, json);
}

void notifyDisplayStatus() {
  if (xSemaphoreTake(g_runtime.mutex, portMAX_DELAY) != pdTRUE)
    return;
  const std::string json = displayStatusJsonLocked();
  const bool notify = g_runtime.displayNotifyEnabled;
  const uint16_t connHandle = g_runtime.connHandle;
  const uint16_t handle = g_runtime.displayStatusHandle;
  xSemaphoreGive(g_runtime.mutex);
  notifyHandle(notify, connHandle, handle, json);
}

esp_err_t ensureRuntimeReady() {
  if (g_runtime.runtimeReady)
    return ESP_OK;

  g_runtime.mutex = xSemaphoreCreateMutex();
  g_runtime.provisioningQueue = xQueueCreate(1, sizeof(WifiCredentials));
  g_runtime.displayQueue = xQueueCreate(2, sizeof(DisplayCommand));
  if (g_runtime.mutex == nullptr || g_runtime.provisioningQueue == nullptr ||
      g_runtime.displayQueue == nullptr) {
    return ESP_ERR_NO_MEM;
  }

  std::string advertisingName = g_runtime.deviceIdentity->name();
  if (advertisingName.empty())
    advertisingName = std::string(config::kProductName) + " " +
                      g_runtime.deviceIdentity->shortIdUpper();

  if (advertisingName.size() > kMaxDeviceNameBytes)
    advertisingName.resize(kMaxDeviceNameBytes);
  g_runtime.advertisingName = advertisingName;
  g_runtime.runtimeReady = true;
  return ESP_OK;
}

esp_err_t ensureBleHostStarted() {
  if (g_runtime.hostStarted)
    return ESP_OK;

  ESP_RETURN_ON_ERROR(ensureRuntimeReady(), TAG, "Failed to prepare BLE runtime");

  ESP_RETURN_ON_ERROR(nimble_port_init(), TAG, "Failed to initialize NimBLE port");
  ble_svc_gap_init();
  ble_svc_gatt_init();
  ble_store_config_init();
  ble_hs_cfg.sync_cb = onSync;
  ble_hs_cfg.reset_cb = onReset;

  int rc = ble_gatts_count_cfg(g_services);
  if (rc != 0)
    return ESP_FAIL;
  rc = ble_gatts_add_svcs(g_services);
  if (rc != 0)
    return ESP_FAIL;

  nimble_port_freertos_init(hostTask);
  if (xTaskCreate(provisioningWorkerTask, "ble_wifi_worker", 6144, nullptr, 4,
                  nullptr) != pdPASS)
    return ESP_ERR_NO_MEM;
  if (xTaskCreate(displayWorkerTask, "ble_display_worker", 6144, nullptr, 4,
                  nullptr) != pdPASS)
    return ESP_ERR_NO_MEM;

  g_runtime.hostStarted = true;
  return ESP_OK;
}

} // namespace

BleProvisioningService::BleProvisioningService(DeviceIdentity &deviceIdentity,
                                               Storage &storage,
                                               WifiManager &wifiManager,
                                               DisplayService &displayService,
                                               MdnsService &mdnsService)
    : m_deviceIdentity(deviceIdentity),
      m_storage(storage),
      m_wifiManager(wifiManager),
      m_displayService(displayService),
      m_mdnsService(mdnsService) {
  g_runtime.deviceIdentity = &m_deviceIdentity;
  g_runtime.storage = &m_storage;
  g_runtime.wifiManager = &m_wifiManager;
  g_runtime.displayService = &m_displayService;
  g_runtime.mdnsService = &m_mdnsService;
}

esp_err_t BleProvisioningService::begin() { return ensureBleHostStarted(); }

BleDisplayService::BleDisplayService(DeviceIdentity &deviceIdentity,
                                     Storage &storage, WifiManager &wifiManager,
                                     DisplayService &displayService)
    : m_deviceIdentity(deviceIdentity),
      m_storage(storage),
      m_wifiManager(wifiManager),
      m_displayService(displayService) {
  g_runtime.deviceIdentity = &m_deviceIdentity;
  g_runtime.storage = &m_storage;
  g_runtime.wifiManager = &m_wifiManager;
  g_runtime.displayService = &m_displayService;
}

esp_err_t BleDisplayService::begin() { return ensureBleHostStarted(); }
