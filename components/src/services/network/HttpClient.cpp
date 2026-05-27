/**
 * @file HttpClient.cpp
 * @brief Primitive HTTP/HTTPS download service.
 */

#include "services/network/HttpClient.hpp"

#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <cstring>

namespace {

constexpr const char *TAG = "ESP_HTTP_CLIENT";

esp_err_t downloadInternal(const char *url, const char *certificate,
                           bool secure, uint8_t **buffer, int32_t *length) {
  if (url == nullptr || buffer == nullptr || length == nullptr || *length <= 0)
    return ESP_ERR_INVALID_ARG;

  *buffer = nullptr;

  esp_http_client_config_t config = {};
  config.url = url;
  config.timeout_ms = 10000;
  if (secure) {
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    config.cert_pem = certificate;
  }

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr)
    return ESP_FAIL;

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return err;
  }

  int32_t contentLength = static_cast<int32_t>(esp_http_client_fetch_headers(client));
  if (contentLength <= 0)
    contentLength = *length;
  else
    *length = contentLength;

  uint8_t *downloadBuffer = static_cast<uint8_t *>(
      heap_caps_malloc(contentLength, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (downloadBuffer == nullptr) {
    downloadBuffer =
        static_cast<uint8_t *>(heap_caps_malloc(contentLength, MALLOC_CAP_8BIT));
  }
  if (downloadBuffer == nullptr) {
    esp_http_client_cleanup(client);
    return ESP_ERR_NO_MEM;
  }

  int32_t totalRead = 0;
  uint8_t chunk[512];
  while (totalRead < contentLength) {
    int32_t remaining = contentLength - totalRead;
    int toRead =
        remaining < static_cast<int32_t>(sizeof(chunk)) ? remaining : sizeof(chunk);
    int read = esp_http_client_read(client, reinterpret_cast<char *>(chunk), toRead);
    if (read <= 0)
      break;

    memcpy(downloadBuffer + totalRead, chunk, read);
    totalRead += read;
  }

  esp_http_client_cleanup(client);
  *buffer = downloadBuffer;
  *length = totalRead;
  return ESP_OK;
}

} // namespace

esp_err_t HttpClient::download(const char *url, uint8_t **buffer,
                               int32_t *length) {
  return downloadInternal(url, nullptr, false, buffer, length);
}

esp_err_t HttpClient::downloadSecure(const char *url, uint8_t **buffer,
                                     int32_t *length) {
  return downloadInternal(url, m_certificate, true, buffer, length);
}
