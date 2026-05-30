/**
 * @file MdnsService.cpp
 * @brief mDNS service wrapper.
 */

#include "services/network/MdnsService.hpp"

#include "config/AppConfig.hpp"
#include "mdns.h"
#include "panels/PanelFactory.hpp"
#include "services/device/DeviceIdentity.hpp"

static const char *HTTP_SERVICE_TYPE = "_http";
static const char *HTTP_SERVICE_PROTO = "_tcp";

esp_err_t MdnsService::begin(const char *hostname, const char *instanceName) {
  if (!m_started) {
    esp_err_t err = mdns_init();
    if (err != ESP_OK)
      return err;

    m_started = true;
  }

  esp_err_t err = mdns_hostname_set(hostname);
  if (err != ESP_OK)
    return err;

  return mdns_instance_name_set(instanceName);
}

esp_err_t MdnsService::begin(const DeviceIdentity &deviceIdentity) {
  return begin(deviceIdentity.hostname().c_str(), deviceIdentity.name().c_str());
}

esp_err_t MdnsService::addHttpService(uint16_t port) {
  if (m_httpServiceAdded)
    return ESP_OK;

  const auto firmware = prostokont::currentFirmwareDescriptor();
  mdns_txt_item_t txt[] = {
      {"model", firmware.model},
      {"fw", firmware.version},
  };

  esp_err_t err = mdns_service_add(nullptr, HTTP_SERVICE_TYPE, HTTP_SERVICE_PROTO,
                                   port, txt, sizeof(txt) / sizeof(txt[0]));
  if (err != ESP_OK)
    return err;

  m_httpServiceAdded = true;
  return ESP_OK;
}
