/**
 * @file HttpServer.cpp
 * @brief Local HTTP API service.
 */

#include "services/http/HttpServer.hpp"

#include "Display.h"
#include "cJSON.h"
#include "config/AppConfig.hpp"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "services/ble/BleProvisioningService.hpp"
#include "services/board/BoardProfile.hpp"
#include "services/device/DeviceIdentity.hpp"
#include "services/display/DisplayService.hpp"
#include "services/network/MdnsService.hpp"
#include "services/storage/Storage.hpp"
#include "services/wifi/WifiManager.hpp"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

constexpr const char *TAG = "ESP_HTTP_SERVER";
constexpr size_t PACKED_FRAME_BYTES = E_INK_WIDTH * E_INK_HEIGHT / 2;

void addString(cJSON *json, const char *key, const char *value) {
  cJSON_AddStringToObject(json, key, value != nullptr ? value : "");
}

} // namespace

HttpServer::HttpServer(DeviceIdentity &deviceIdentity, Storage &storage,
                       WifiManager &wifiManager,
                       BleProvisioningService &bleProvisioningService,
                       DisplayService &displayService, MdnsService &mdnsService)
    : m_deviceIdentity(deviceIdentity),
      m_storage(storage),
      m_wifiManager(wifiManager),
      m_bleProvisioningService(bleProvisioningService),
      m_displayService(displayService),
      m_mdnsService(mdnsService) {}

esp_err_t HttpServer::begin() {
  if (m_server != nullptr)
    return ESP_OK;

  httpd_config_t serverConfig = HTTPD_DEFAULT_CONFIG();
  serverConfig.server_port = config::kHttpPort;
  serverConfig.max_uri_handlers = 16;

  ESP_RETURN_ON_ERROR(httpd_start(&m_server, &serverConfig), TAG,
                      "Failed to start HTTP server");
  ESP_RETURN_ON_ERROR(registerRoutes(), TAG, "Failed to register routes");

  ESP_LOGI(TAG, "HTTP API listening on port %d", config::kHttpPort);
  return ESP_OK;
}

esp_err_t HttpServer::registerRoutes() {
  struct Route {
    const char *uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t *req);
  };

  const Route routes[] = {
      {config::kDiscoveryPath, HTTP_GET, discoveryHandler},
      {config::kStatusPath, HTTP_GET, statusHandler},
      {config::kWifiScanPath, HTTP_POST, wifiScanHandler},
      {config::kWifiConfigurePath, HTTP_POST, wifiConfigureHandler},
      {config::kWifiResetPath, HTTP_POST, wifiResetHandler},
      {config::kImagePath, HTTP_POST, imageHandler},
      {config::kImagePath, HTTP_DELETE, imageClearHandler},
      {config::kDiscoveryPath, HTTP_OPTIONS, optionsHandler},
      {config::kStatusPath, HTTP_OPTIONS, optionsHandler},
      {config::kWifiScanPath, HTTP_OPTIONS, optionsHandler},
      {config::kWifiConfigurePath, HTTP_OPTIONS, optionsHandler},
      {config::kWifiResetPath, HTTP_OPTIONS, optionsHandler},
      {config::kImagePath, HTTP_OPTIONS, optionsHandler},
  };

  for (const auto &route : routes) {
    httpd_uri_t uri = {};
    uri.uri = route.uri;
    uri.method = route.method;
    uri.handler = route.handler;
    uri.user_ctx = this;
    ESP_RETURN_ON_ERROR(httpd_register_uri_handler(m_server, &uri), TAG,
                        "Failed to register route");
  }

  return ESP_OK;
}

esp_err_t HttpServer::optionsHandler(httpd_req_t *req) {

  httpd_resp_set_status(req, "204 No Content");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",

                     "GET, POST, DELETE, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers",

                     "Content-Type, Authorization, X-Requested-With, Accept, Origin");
  httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, nullptr, 0);
}

esp_err_t HttpServer::discoveryHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  cJSON *json = cJSON_CreateObject();
  addString(json, "device_id", server->m_deviceIdentity.deviceId().c_str());
  addString(json, "name", server->m_deviceIdentity.name().c_str());
  addString(json, "model", config::kModel);
  addString(json, "board", currentBoardProfile().key);

  cJSON *capabilities = cJSON_AddArrayToObject(json, "capabilities");
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("wifi-provisioning"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("ble-display-v1"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("image-upload"));
  cJSON_AddItemToArray(capabilities, cJSON_CreateString("image-clear"));

  char apiBase[128];
  std::snprintf(apiBase, sizeof(apiBase), "http://%s.local%s",
                server->m_deviceIdentity.hostname().c_str(),
                config::kApiBasePath);
  addString(json, "api", apiBase);

  esp_err_t err = server->sendJson(req, json);
  cJSON_Delete(json);
  return err;
}

esp_err_t HttpServer::statusHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  cJSON *json = cJSON_CreateObject();
  addString(json, "device_id", server->m_deviceIdentity.deviceId().c_str());
  addString(json, "name", server->m_deviceIdentity.name().c_str());
  addString(json, "hostname", server->m_deviceIdentity.hostname().c_str());
  addString(json, "model", config::kModel);
  addString(json, "board", currentBoardProfile().key);
  addString(json, "firmware_version", config::kFirmwareVersion);
  cJSON_AddBoolToObject(json, "wifi_provisioned",
                        server->m_storage.isWifiProvisioned());
  cJSON_AddBoolToObject(json, "sta_connected",
                        server->m_wifiManager.hasStaIp());
  cJSON_AddNumberToObject(json, "display_width", currentBoardProfile().width);
  cJSON_AddNumberToObject(json, "display_height", currentBoardProfile().height);
  cJSON_AddBoolToObject(json, "packed_frame_supported",
                        currentBoardProfile().packedFrameSupported);
  addString(json, "color_scheme", currentBoardProfile().colorScheme);

  esp_err_t err = server->sendJson(req, json);
  cJSON_Delete(json);
  return err;
}

esp_err_t HttpServer::wifiScanHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);
  return server->sendJson(req, server->m_wifiManager.scanJson().c_str());
}

esp_err_t HttpServer::wifiConfigureHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  char *body = server->readBody(req, config::kMaxJsonBodyBytes);
  if (body == nullptr)
    return server->sendJson(req, "{\"error\":\"invalid_json\"}",
                            "400 Bad Request");

  cJSON *json = cJSON_Parse(body);
  heap_caps_free(body);
  if (json == nullptr)
    return server->sendJson(req, "{\"error\":\"invalid_json\"}",
                            "400 Bad Request");

  const cJSON *ssidItem = cJSON_GetObjectItemCaseSensitive(json, "ssid");
  const cJSON *passwordItem =
      cJSON_GetObjectItemCaseSensitive(json, "password");
  if (!cJSON_IsString(ssidItem) || ssidItem->valuestring == nullptr ||
      ssidItem->valuestring[0] == '\0') {
    cJSON_Delete(json);
    return server->sendJson(req, "{\"error\":\"ssid_required\"}",
                            "400 Bad Request");
  }

  const char *password = "";
  if (cJSON_IsString(passwordItem) && passwordItem->valuestring != nullptr)
    password = passwordItem->valuestring;

  esp_err_t err = server->m_wifiManager.configureAndConnect(
      ssidItem->valuestring, password, server->m_deviceIdentity.hostname());
  cJSON_Delete(json);
  if (err != ESP_OK)
    return server->sendJson(req, "{\"error\":\"wifi_config_failed\"}",
                            "500 Internal Server Error");

  ESP_ERROR_CHECK_WITHOUT_ABORT(server->m_storage.setWifiProvisioned(true));
  if (!server->m_wifiManager.waitForStaIp(
          pdMS_TO_TICKS(config::kStaConnectTimeoutMs)))
    return server->sendJson(req, "{\"state\":\"connecting\"}");

  err = server->ensureMdnsReady();
  if (err != ESP_OK)
    ESP_LOGW(TAG, "mDNS init failed after WiFi configure: %s",
             esp_err_to_name(err));

  std::string readyUrl = "http://" + server->m_deviceIdentity.hostname() + ".local";
  ESP_ERROR_CHECK_WITHOUT_ABORT(
      server->m_displayService.showReady(readyUrl.c_str()));

  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "state", "connected");
  addString(response, "hostname", server->m_deviceIdentity.hostname().c_str());

  char apiBase[128];
  std::snprintf(apiBase, sizeof(apiBase), "http://%s.local%s",
                server->m_deviceIdentity.hostname().c_str(),
                config::kApiBasePath);
  addString(response, "api", apiBase);

  err = server->sendJson(req, response);
  cJSON_Delete(response);
  return err;
}

esp_err_t HttpServer::wifiResetHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  esp_err_t err = server->m_wifiManager.clearSavedCredentials();
  if (err != ESP_OK)
    return server->sendJson(req, "{\"reset\":false,\"error\":\"wifi_reset_failed\"}",
                            "500 Internal Server Error");

  err = server->m_storage.setWifiProvisioned(false);
  if (err != ESP_OK)
    return server->sendJson(req, "{\"reset\":false,\"error\":\"storage_update_failed\"}",
                            "500 Internal Server Error");

  const std::string apSsid =
      std::string(config::kSoftApSsidPrefix) + server->m_deviceIdentity.shortIdUpper();
  err = server->m_wifiManager.startSetupSoftAp(apSsid);
  if (err != ESP_OK)
    return server->sendJson(req, "{\"reset\":false,\"error\":\"setup_ap_failed\"}",
                            "500 Internal Server Error");

  err = server->m_bleProvisioningService.begin();
  if (err != ESP_OK)
    return server->sendJson(req, "{\"reset\":false,\"error\":\"ble_setup_failed\"}",
                            "500 Internal Server Error");

  ESP_ERROR_CHECK_WITHOUT_ABORT(server->m_displayService.showSetupScreen(
      server->m_deviceIdentity, apSsid.c_str(), config::kSoftApUrl, nullptr,
      server->m_deviceIdentity.name().c_str()));

  cJSON *response = cJSON_CreateObject();
  cJSON_AddBoolToObject(response, "reset", true);
  cJSON_AddBoolToObject(response, "wifi_provisioned", false);
  cJSON_AddStringToObject(response, "state", "setup");
  addString(response, "ap_ssid", apSsid.c_str());
  addString(response, "ap_url", config::kSoftApUrl);

  err = server->sendJson(req, response);
  cJSON_Delete(response);
  return err;
}

esp_err_t HttpServer::imageHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  esp_err_t err = server->renderRequestImage(req);
  if (err == ESP_ERR_INVALID_SIZE)
    return server->sendJson(
        req, "{\"accepted\":false,\"error\":\"invalid_image_size\"}",
        "400 Bad Request");
  if (err == ESP_ERR_NOT_SUPPORTED)
    return server->sendJson(
        req,
        "{\"accepted\":false,\"error\":\"unsupported_image_format\","
        "\"supported_formats\":[\"jpeg\",\"png\",\"bmp\"]}",
        "415 Unsupported Media Type");
  if (err == ESP_ERR_NO_MEM)
    return server->sendJson(req,
                            "{\"accepted\":false,\"error\":\"out_of_memory\"}",
                            "507 Insufficient Storage");
  if (err != ESP_OK)
    return server->sendJson(
        req, "{\"accepted\":false,\"error\":\"image_render_failed\"}",
        "500 Internal Server Error");

  return server->sendJson(req, "{\"accepted\":true}");
}

esp_err_t HttpServer::imageClearHandler(httpd_req_t *req) {
  auto *server = static_cast<HttpServer *>(req->user_ctx);

  esp_err_t err = server->m_displayService.clear();
  if (err != ESP_OK)
    return server->sendJson(
        req, "{\"cleared\":false,\"error\":\"display_clear_failed\"}",
        "500 Internal Server Error");

  return server->sendJson(req, "{\"cleared\":true}");
}

esp_err_t HttpServer::sendJson(httpd_req_t *req, const char *json,
                               const char *status) const {
  if (status != nullptr)
    httpd_resp_set_status(req, status);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
                     "GET, POST, DELETE, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers",
                     "Content-Type, Authorization, X-Requested-With, Accept, Origin");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
  return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

esp_err_t HttpServer::sendJson(httpd_req_t *req, const cJSON *json,
                               const char *status) const {
  char *printed = cJSON_PrintUnformatted(json);
  if (printed == nullptr)
    return ESP_ERR_NO_MEM;

  esp_err_t err = sendJson(req, printed, status);
  cJSON_free(printed);
  return err;
}

char *HttpServer::readBody(httpd_req_t *req, size_t maxLen) const {
  size_t bodyLen = static_cast<size_t>(req->content_len);
  if (bodyLen == 0 || bodyLen > maxLen)
    return nullptr;

  char *buffer =
      static_cast<char *>(heap_caps_malloc(bodyLen + 1, MALLOC_CAP_8BIT));
  if (buffer == nullptr)
    return nullptr;

  size_t received = 0;
  while (received < bodyLen) {
    int chunk =
        httpd_req_recv(req, buffer + received, static_cast<int>(bodyLen - received));
    if (chunk <= 0) {
      heap_caps_free(buffer);
      return nullptr;
    }
    received += static_cast<size_t>(chunk);
  }

  buffer[received] = '\0';
  return buffer;
}

esp_err_t HttpServer::ensureMdnsReady() {
  ESP_RETURN_ON_ERROR(m_mdnsService.begin(m_deviceIdentity), TAG,
                      "Failed to initialize mDNS");
  return m_mdnsService.addHttpService();
}

esp_err_t HttpServer::renderRequestImage(httpd_req_t *req) {
  size_t bodyLen = static_cast<size_t>(req->content_len);
  if (bodyLen == 0 || bodyLen > config::kMaxImageUploadBytes)
    return ESP_ERR_INVALID_SIZE;

  uint8_t *body = static_cast<uint8_t *>(
      heap_caps_malloc(bodyLen, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (body == nullptr)
    body = static_cast<uint8_t *>(heap_caps_malloc(bodyLen, MALLOC_CAP_8BIT));
  if (body == nullptr)
    return ESP_ERR_NO_MEM;

  size_t received = 0;
  while (received < bodyLen) {
    int chunk = httpd_req_recv(req, reinterpret_cast<char *>(body + received),
                               static_cast<int>(bodyLen - received));
    if (chunk <= 0) {
      heap_caps_free(body);
      return ESP_FAIL;
    }
    received += static_cast<size_t>(chunk);
  }

  esp_err_t err = ESP_OK;
  if (bodyLen == PACKED_FRAME_BYTES)
    err = m_displayService.showPackedFrame(body, PACKED_FRAME_BYTES);
  else
    err = m_displayService.showImage(body, bodyLen, true);

  heap_caps_free(body);
  return err;
}
