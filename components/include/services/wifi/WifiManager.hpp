/**
 * @file WifiManager.hpp
 * @brief Application-level Wi-Fi manager.
 */

#pragma once

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <string>

class WifiManager {
public:
  WifiManager() = default;
  ~WifiManager();

  WifiManager(const WifiManager &) = delete;
  WifiManager &operator=(const WifiManager &) = delete;

  esp_err_t begin();
  esp_err_t startSetupSoftAp(const std::string &ssid);
  esp_err_t connectSaved(const std::string &hostname);
  esp_err_t configureAndConnect(const std::string &ssid,
                                const std::string &password,
                                const std::string &hostname);
  esp_err_t clearSavedCredentials();

  bool waitForStaIp(TickType_t timeoutTicks) const;
  bool hasStaIp() const;
  std::string scanJson();

  const std::string &softApSsid() const { return m_softApSsid; }

private:
  static void eventHandler(void *arg, esp_event_base_t eventBase,
                           int32_t eventId, void *eventData);
  void onEvent(esp_event_base_t eventBase, int32_t eventId, void *eventData);
  esp_err_t startWifiIfNeeded();

  esp_netif_t *m_staNetif = nullptr;
  esp_netif_t *m_apNetif = nullptr;
  EventGroupHandle_t m_eventGroup = nullptr;
  bool m_started = false;
  std::string m_softApSsid;
};
