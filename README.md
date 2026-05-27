# Prostokont Firmware

Firmware for the Prostokont e-paper frame built on ESP-IDF.

This repository contains the full device firmware, including:

- display drivers for Inkplate and Waveshare panels
- Wi-Fi provisioning over BLE and fallback SoftAP
- a local HTTP API for discovery, status, Wi-Fi setup, and image upload
- mDNS advertising for `.local` access on the LAN
- persistent device settings stored in NVS

The current checked-in configuration targets `esp32s3` with `CONFIG_WAVESHARE_BOARD_WAVESHARE13=y`.

## Repository Layout

```text
.
├── CMakeLists.txt                # Project entry point
├── components/                   # Shared firmware component
│   ├── include/                  # Public headers
│   │   ├── config/               # Compile-time app configuration
│   │   ├── features/             # Hardware feature wrappers
│   │   ├── graphics/             # Drawing and image decoding
│   │   ├── panels/               # Panel-specific interfaces
│   │   └── services/             # Provisioning, HTTP, storage, Wi-Fi, mDNS
│   ├── src/                      # Component implementation
│   │   ├── features/
│   │   ├── graphics/
│   │   ├── panels/
│   │   └── services/
│   ├── CMakeLists.txt            # Component build rules
│   └── Kconfig.projbuild         # Board selection in menuconfig
├── main/
│   ├── CMakeLists.txt            # App component registration
│   └── main.cpp                  # Firmware bootstrap
├── managed_components/           # ESP-IDF managed dependencies
├── sdkconfig.defaults            # Default SDK configuration
└── dependencies.lock             # Locked component dependency versions
```

## Requirements

- ESP-IDF `v6.0` or newer
- ESP-IDF toolchain initialized in your shell
- a supported ESP32 board connected over USB

Follow the official ESP-IDF installation guide for your platform:
https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/

## Build

Initialize the ESP-IDF environment first, then build:

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
```

In `menuconfig`, select the panel under the board configuration menu if you are not using the default checked-in setup.

## Flash

```bash
idf.py -p /dev/tty.usbmodemXXXX flash monitor
```

Replace `/dev/tty.usbmodemXXXX` with the actual serial device for your board.

## Boot and Provisioning Flow

On boot, the firmware:

1. initializes the display
2. loads persistent settings from NVS
3. starts the local HTTP server
4. tries to connect to the saved Wi-Fi network
5. falls back to setup mode if Wi-Fi is missing or unavailable

Setup mode exposes two provisioning transports:

- BLE provisioning service
- SoftAP with an SSID in the form `Prostokont-<SHORT_ID>`

When the device is in SoftAP mode, the setup URL is:

```text
http://192.168.4.1
```

Once connected to the local network, the device advertises itself over mDNS and becomes available at:

```text
http://<hostname>.local
```

## HTTP API

Base paths:

- discovery: `/.well-known/prostokont`
- API root: `/api/v1`

Available endpoints:

| Method | Path | Purpose |
| --- | --- | --- |
| `GET` | `/.well-known/prostokont` | Device discovery payload |
| `GET` | `/api/v1/status` | Current device and network status |
| `POST` | `/api/v1/wifi/scan` | Scan nearby Wi-Fi networks |
| `POST` | `/api/v1/wifi/configure` | Save Wi-Fi credentials and connect |
| `POST` | `/api/v1/wifi/reset` | Clear saved Wi-Fi and return to setup mode |
| `POST` | `/api/v1/image` | Render an uploaded image |
| `DELETE` | `/api/v1/image` | Clear the display |

### Wi-Fi Configure Request

```json
{
  "ssid": "YourNetwork",
  "password": "YourPassword"
}
```

### Image Upload Notes

`POST /api/v1/image` accepts:

- JPEG
- PNG
- BMP
- a packed raw frame when the payload size matches the active panel frame buffer

The current upload limit is 6 MiB.

## Configuration

Application-level constants live in:

- `components/include/config/AppConfig.hpp`

This file defines firmware identity, API paths, setup-mode defaults, storage keys, and other compile-time settings.

## Development Notes

- Do not commit `build/` artifacts.
- Board-specific source selection is controlled by `components/CMakeLists.txt` and `menuconfig`.
- The main application entry point is `main/main.cpp`.
- Device state is persisted under the NVS namespace `prostokont`.

## License

This repository is licensed under the terms of the `GPL-3.0-only` license. See [LICENSE](LICENSE).
