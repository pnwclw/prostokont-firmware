/**
 * @file HttpServer.hpp
 * @brief Local HTTP API service.
 */

#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

struct cJSON;

class DeviceIdentity;
class BleProvisioningService;
class DisplayService;
class MdnsService;
class Storage;
class WifiManager;

class HttpServer {
public:
  HttpServer(DeviceIdentity &deviceIdentity, Storage &storage,
             WifiManager &wifiManager,
             BleProvisioningService &bleProvisioningService,
             DisplayService &displayService, MdnsService &mdnsService);

  esp_err_t begin();

private:
  static esp_err_t optionsHandler(httpd_req_t *req);
  static esp_err_t discoveryHandler(httpd_req_t *req);
  static esp_err_t statusHandler(httpd_req_t *req);
  static esp_err_t wifiScanHandler(httpd_req_t *req);
  static esp_err_t wifiConfigureHandler(httpd_req_t *req);
  static esp_err_t wifiResetHandler(httpd_req_t *req);
  static esp_err_t imageHandler(httpd_req_t *req);
  static esp_err_t imageClearHandler(httpd_req_t *req);

  esp_err_t registerRoutes();
  esp_err_t sendJson(httpd_req_t *req, const char *json,
                     const char *status = nullptr) const;
  esp_err_t sendJson(httpd_req_t *req, const cJSON *json,
                     const char *status = nullptr) const;
  char *readBody(httpd_req_t *req, size_t maxLen) const;
  esp_err_t ensureMdnsReady();
  esp_err_t renderRequestImage(httpd_req_t *req);

  DeviceIdentity &m_deviceIdentity;
  Storage &m_storage;
  WifiManager &m_wifiManager;
  BleProvisioningService &m_bleProvisioningService;
  DisplayService &m_displayService;
  MdnsService &m_mdnsService;
  httpd_handle_t m_server = nullptr;
};
