/**
 * @file Storage.cpp
 * @brief Persistent application storage backed by NVS.
 */

#include "services/storage/Storage.hpp"

#include "config/AppConfig.hpp"
#include "esp_log.h"
#include "nvs_flash.h"

#include <vector>

static const char *TAG = "ESP_STORAGE";

namespace {

std::string getStringForHandle(nvs_handle_t handle, const char *key) {
  size_t len = 0;
  esp_err_t err = nvs_get_str(handle, key, nullptr, &len);
  if (err != ESP_OK || len == 0)
    return {};

  std::vector<char> buffer(len);
  err = nvs_get_str(handle, key, buffer.data(), &len);
  if (err != ESP_OK)
    return {};

  return std::string(buffer.data());
}

esp_err_t commitStringIfMissing(nvs_handle_t dstHandle, const char *key,
                                const std::string &value) {
  if (value.empty())
    return ESP_OK;

  size_t currentLen = 0;
  if (nvs_get_str(dstHandle, key, nullptr, &currentLen) == ESP_OK && currentLen > 0)
    return ESP_OK;

  esp_err_t err = nvs_set_str(dstHandle, key, value.c_str());
  if (err == ESP_OK)
    err = nvs_commit(dstHandle);

  return err;
}

esp_err_t commitU8IfMissing(nvs_handle_t dstHandle, const char *key, uint8_t value,
                            bool hasValue) {
  if (!hasValue)
    return ESP_OK;

  uint8_t current = 0;
  if (nvs_get_u8(dstHandle, key, &current) == ESP_OK)
    return ESP_OK;

  esp_err_t err = nvs_set_u8(dstHandle, key, value);
  if (err == ESP_OK)
    err = nvs_commit(dstHandle);

  return err;
}

esp_err_t migrateLegacyNamespace(nvs_handle_t dstHandle) {
  nvs_handle_t legacyHandle = 0;
  esp_err_t err = nvs_open(config::kLegacyStorageNamespace, NVS_READONLY,
                           &legacyHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND)
    return ESP_OK;
  if (err != ESP_OK)
    return err;

  uint8_t wifiProvisioned = 0;
  bool hasWifiProvisioned =
      nvs_get_u8(legacyHandle, config::kStorageKeyWifiProvisioned,
                 &wifiProvisioned) == ESP_OK;

  err = commitU8IfMissing(dstHandle, config::kStorageKeyWifiProvisioned,
                          wifiProvisioned, hasWifiProvisioned);
  if (err == ESP_OK) {
    err = commitStringIfMissing(dstHandle, config::kStorageKeyPairingToken,
                                getStringForHandle(
                                    legacyHandle, config::kStorageKeyPairingToken));
  }
  if (err == ESP_OK) {
    err = commitStringIfMissing(
        dstHandle, config::kStorageKeyDeviceId,
        getStringForHandle(legacyHandle, config::kStorageKeyDeviceId));
  }
  if (err == ESP_OK) {
    err = commitStringIfMissing(
        dstHandle, config::kStorageKeyDeviceName,
        getStringForHandle(legacyHandle, config::kStorageKeyDeviceName));
  }

  nvs_close(legacyHandle);
  return err;
}

} // namespace

Storage::~Storage() {
  if (m_handle != 0)
    nvs_close(m_handle);
}

esp_err_t Storage::begin() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  if (err != ESP_OK)
    return err;

  err = nvs_open(config::kStorageNamespace, NVS_READWRITE, &m_handle);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "NVS namespace opened: %s", config::kStorageNamespace);
    err = migrateLegacyNamespace(m_handle);
  }

  return err;
}

bool Storage::isWifiProvisioned() const {
  uint8_t value = 0;
  if (nvs_get_u8(m_handle, config::kStorageKeyWifiProvisioned, &value) != ESP_OK)
    return false;

  return value != 0;
}

esp_err_t Storage::setWifiProvisioned(bool provisioned) {
  esp_err_t err = nvs_set_u8(m_handle, config::kStorageKeyWifiProvisioned,
                             provisioned ? 1 : 0);
  if (err == ESP_OK)
    err = nvs_commit(m_handle);

  return err;
}

std::string Storage::pairingToken() const {
  return getString(config::kStorageKeyPairingToken);
}

esp_err_t Storage::setPairingToken(const std::string &token) {
  return setString(config::kStorageKeyPairingToken, token);
}

bool Storage::hasPairingToken() const { return !pairingToken().empty(); }

std::string Storage::deviceId() const {
  return getString(config::kStorageKeyDeviceId);
}

esp_err_t Storage::setDeviceId(const std::string &deviceId) {
  return setString(config::kStorageKeyDeviceId, deviceId);
}

std::string Storage::deviceName() const {
  return getString(config::kStorageKeyDeviceName);
}

esp_err_t Storage::setDeviceName(const std::string &name) {
  return setString(config::kStorageKeyDeviceName, name);
}

std::string Storage::getString(const char *key) const {
  size_t len = 0;
  esp_err_t err = nvs_get_str(m_handle, key, nullptr, &len);
  if (err != ESP_OK || len == 0)
    return {};

  std::vector<char> buffer(len);
  err = nvs_get_str(m_handle, key, buffer.data(), &len);
  if (err != ESP_OK)
    return {};

  return std::string(buffer.data());
}

esp_err_t Storage::setString(const char *key, const std::string &value) {
  esp_err_t err = nvs_set_str(m_handle, key, value.c_str());
  if (err == ESP_OK)
    err = nvs_commit(m_handle);

  return err;
}
