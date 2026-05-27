/**
 * @file HttpClient.hpp
 * @brief Primitive HTTP/HTTPS download service.
 */

#pragma once

#include "esp_err.h"
#include "stdint.h"

class HttpClient {
public:
  HttpClient() = default;

  void setCertificate(const char *certificate) { m_certificate = certificate; }

  esp_err_t download(const char *url, uint8_t **buffer, int32_t *length);
  esp_err_t downloadSecure(const char *url, uint8_t **buffer, int32_t *length);

private:
  const char *m_certificate = nullptr;
};
