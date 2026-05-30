/**
 * @file WifiManager.cpp
 * @brief Application-level Wi-Fi manager.
 */

#include "services/wifi/WifiManager.hpp"

#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include <algorithm>
#include <cstring>
#include <vector>

static const char *TAG = "ESP_WIFI_MANAGER";

static constexpr EventBits_t WIFI_STA_GOT_IP = BIT0;
static constexpr EventBits_t WIFI_STA_DISCONNECTED = BIT1;
static constexpr EventBits_t WIFI_STA_CHANGED = BIT2;

static const char *authModeName(wifi_auth_mode_t mode) {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return "open";
  case WIFI_AUTH_WEP:
    return "wep";
  case WIFI_AUTH_WPA_PSK:
    return "wpa";
  case WIFI_AUTH_WPA2_PSK:
    return "wpa2";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "wpa_wpa2";
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "wpa2_enterprise";
  case WIFI_AUTH_WPA3_PSK:
    return "wpa3";
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return "wpa2_wpa3";
  default:
    return "unknown";
  }
}

static std::string jsonEscape(const char *value) {
  std::string out;
  for (const char *p = value; p != nullptr && *p != '\0'; ++p) {
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

WifiManager::~WifiManager() {
  if (m_eventGroup != nullptr)
    vEventGroupDelete(m_eventGroup);
}

esp_err_t WifiManager::begin() {
  esp_err_t err = esp_netif_init();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    return err;

  err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    return err;

  m_eventGroup = xEventGroupCreate();
  if (m_eventGroup == nullptr)
    return ESP_ERR_NO_MEM;

  m_staNetif = esp_netif_create_default_wifi_sta();
  m_apNetif = esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialize Wi-Fi");
  ESP_RETURN_ON_ERROR(esp_wifi_set_storage(WIFI_STORAGE_FLASH), TAG,
                      "Failed to set Wi-Fi storage");
  ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(
                          WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::eventHandler,
                          this, nullptr),
                      TAG, "Failed to register Wi-Fi handler");
  ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(
                          IP_EVENT, IP_EVENT_STA_GOT_IP,
                          &WifiManager::eventHandler, this, nullptr),
                      TAG, "Failed to register IP handler");

  return ESP_OK;
}

esp_err_t WifiManager::startSetupSoftAp(const std::string &ssid) {
  m_softApSsid = ssid;

  wifi_config_t apConfig = {};
  std::strncpy(reinterpret_cast<char *>(apConfig.ap.ssid), ssid.c_str(),
               sizeof(apConfig.ap.ssid) - 1);
  apConfig.ap.ssid_len = ssid.size();
  apConfig.ap.channel = config::kSoftApChannel;
  apConfig.ap.max_connection = config::kSoftApMaxConnections;
  apConfig.ap.authmode = WIFI_AUTH_OPEN;
  apConfig.ap.pmf_cfg.required = false;

  ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_APSTA), TAG,
                      "Failed to set APSTA mode");
  ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_AP, &apConfig), TAG,
                      "Failed to set SoftAP config");
  ESP_RETURN_ON_ERROR(startWifiIfNeeded(), TAG, "Failed to start Wi-Fi");

  ESP_LOGI(TAG, "SoftAP setup mode started: %s", ssid.c_str());
  return ESP_OK;
}

esp_err_t WifiManager::connectSaved(const std::string &hostname) {
  if (m_staNetif != nullptr) {
    ESP_RETURN_ON_ERROR(esp_netif_set_hostname(m_staNetif, hostname.c_str()),
                        TAG, "Failed to set hostname");
  }

  xEventGroupClearBits(m_eventGroup, WIFI_STA_GOT_IP | WIFI_STA_DISCONNECTED);
  ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG,
                      "Failed to set STA mode");
  ESP_RETURN_ON_ERROR(startWifiIfNeeded(), TAG, "Failed to start Wi-Fi");
  ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "Failed to connect Wi-Fi");
  return ESP_OK;
}

esp_err_t WifiManager::configureAndConnect(const std::string &ssid,
                                           const std::string &password,
                                           const std::string &hostname) {
  wifi_config_t staConfig = {};
  std::strncpy(reinterpret_cast<char *>(staConfig.sta.ssid), ssid.c_str(),
               sizeof(staConfig.sta.ssid) - 1);
  std::strncpy(reinterpret_cast<char *>(staConfig.sta.password),
               password.c_str(), sizeof(staConfig.sta.password) - 1);
  staConfig.sta.threshold.authmode =
      password.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
  staConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

  if (m_staNetif != nullptr) {
    ESP_RETURN_ON_ERROR(esp_netif_set_hostname(m_staNetif, hostname.c_str()),
                        TAG, "Failed to set hostname");
  }

  xEventGroupClearBits(m_eventGroup, WIFI_STA_GOT_IP | WIFI_STA_DISCONNECTED);
  ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_APSTA), TAG,
                      "Failed to set APSTA mode");
  ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &staConfig), TAG,
                      "Failed to set STA config");
  ESP_RETURN_ON_ERROR(startWifiIfNeeded(), TAG, "Failed to start Wi-Fi");
  ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG,
                      "Failed to start STA connection");

  ESP_LOGI(TAG, "Wi-Fi credentials accepted for SSID '%s'", ssid.c_str());
  return ESP_OK;
}

esp_err_t WifiManager::clearSavedCredentials() {
  xEventGroupClearBits(m_eventGroup, WIFI_STA_GOT_IP | WIFI_STA_DISCONNECTED);

  esp_err_t err = esp_wifi_disconnect();
  if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED &&
      err != ESP_ERR_WIFI_NOT_INIT)
    return err;

  return esp_wifi_restore();
}

bool WifiManager::waitForStaIp(TickType_t timeoutTicks) const {
  EventBits_t bits = xEventGroupWaitBits(m_eventGroup, WIFI_STA_GOT_IP, pdFALSE,
                                         pdFALSE, timeoutTicks);
  return (bits & WIFI_STA_GOT_IP) != 0;
}

bool WifiManager::hasStaIp() const {
  EventBits_t bits = xEventGroupGetBits(m_eventGroup);
  return (bits & WIFI_STA_GOT_IP) != 0;
}

void WifiManager::waitForConnectivityChange(TickType_t timeoutTicks) const {
  xEventGroupWaitBits(m_eventGroup, WIFI_STA_CHANGED, pdTRUE, pdFALSE,
                      timeoutTicks);
}

std::string WifiManager::scanJson() {
  wifi_scan_config_t scanConfig = {};
  esp_err_t err = esp_wifi_scan_start(&scanConfig, true);
  if (err != ESP_OK)
    return "{\"networks\":[],\"error\":\"scan_failed\"}";

  uint16_t apCount = 0;
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_get_ap_num(&apCount));
  apCount = std::min<uint16_t>(apCount, 20);

  std::vector<wifi_ap_record_t> records(apCount);
  if (apCount > 0)
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        esp_wifi_scan_get_ap_records(&apCount, records.data()));

  std::string json = "{\"networks\":[";
  for (uint16_t i = 0; i < apCount; ++i) {
    if (i > 0)
      json += ",";

    json += "{\"ssid\":\"";
    json += jsonEscape(reinterpret_cast<const char *>(records[i].ssid));
    json += "\",\"rssi\":";
    json += std::to_string(records[i].rssi);
    json += ",\"auth\":\"";
    json += authModeName(records[i].authmode);
    json += "\"}";
  }
  json += "]}";

  return json;
}

void WifiManager::eventHandler(void *arg, esp_event_base_t eventBase,
                               int32_t eventId, void *eventData) {
  static_cast<WifiManager *>(arg)->onEvent(eventBase, eventId, eventData);
}

void WifiManager::onEvent(esp_event_base_t eventBase, int32_t eventId,
                          void *eventData) {
  if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
    xEventGroupClearBits(m_eventGroup, WIFI_STA_GOT_IP);
    xEventGroupSetBits(m_eventGroup, WIFI_STA_DISCONNECTED | WIFI_STA_CHANGED);
    ESP_LOGW(TAG, "STA disconnected");
  } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
    auto *event = static_cast<ip_event_got_ip_t *>(eventData);
    xEventGroupSetBits(m_eventGroup, WIFI_STA_GOT_IP | WIFI_STA_CHANGED);
    ESP_LOGI(TAG, "STA got IP: " IPSTR, IP2STR(&event->ip_info.ip));
  }
}

esp_err_t WifiManager::startWifiIfNeeded() {
  if (m_started)
    return ESP_OK;

  ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start Wi-Fi");
  m_started = true;
  return ESP_OK;
}
