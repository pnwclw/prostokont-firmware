/**
 * @file AppConfig.hpp
 * @brief Compile-time application configuration.
 */

#pragma once

#include <cstddef>

namespace config {

inline constexpr const char *kFirmwareVersion = "0.1.0-dev";
inline constexpr const char *kProductName = "Prostokont";
inline constexpr const char *kModel = "prostokont-waveshare-13.3e6";
inline constexpr const char *kHostnamePrefix = "prostokont-";
inline constexpr const char *kSoftApSsidPrefix = "Prostokont-";
inline constexpr const char *kDeviceIdPrefix = "pk_";

inline constexpr int kHttpPort = 80;
inline constexpr int kSoftApChannel = 6;
inline constexpr int kSoftApMaxConnections = 4;
inline constexpr int kStaConnectTimeoutMs = 30000;
inline constexpr const char *kSoftApUrl = "http://192.168.4.1";
inline constexpr const char *kCorsAllowedOrigin = "*";
inline constexpr const char *kCorsAllowedMethods = "GET, POST, DELETE, OPTIONS";
inline constexpr const char *kCorsAllowedHeaders =
    "Content-Type, Authorization, X-Requested-With, Accept, Origin";

inline constexpr const char *kApiBasePath = "/api/v1";
inline constexpr const char *kDiscoveryPath = "/.well-known/prostokont";
inline constexpr const char *kStatusPath = "/api/v1/status";
inline constexpr const char *kWifiScanPath = "/api/v1/wifi/scan";
inline constexpr const char *kWifiConfigurePath = "/api/v1/wifi/configure";
inline constexpr const char *kWifiResetPath = "/api/v1/wifi/reset";
inline constexpr const char *kImagePath = "/api/v1/image";
inline constexpr size_t kMaxJsonBodyBytes = 1024;
inline constexpr size_t kMaxImageUploadBytes = 6 * 1024 * 1024;

inline constexpr const char *kStorageNamespace = "prostokont";
inline constexpr const char *kStorageKeyWifiProvisioned = "wifi_ok";
inline constexpr const char *kStorageKeyDeviceId = "device_id";
inline constexpr const char *kStorageKeyDeviceName = "dev_name";

} // namespace config
