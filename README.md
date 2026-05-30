# Prostokont Firmware

Firmware for the Prostokont e-paper frame built on ESP-IDF.

This repository contains the full device firmware, including:

- display drivers for Inkplate and Waveshare panels
- Wi-Fi provisioning over BLE and fallback SoftAP
- a BLE display transport for image upload and display control
- a local HTTP API for discovery, status, Wi-Fi setup, and image upload
- mDNS advertising for `.local` access on the LAN
- onboarding screens with QR codes for setup and local access
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

### Default (checked-in target)

Initialize the ESP-IDF environment first, then build:

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
```

In `menuconfig`, select the panel under the board configuration menu if you are
not using the default checked-in setup.

### Reproducible per-board builds

Each supported (board, panel) pair has a defaults overlay under `boards/` and
an entry in `boards/manifest.json` that maps it to its IDF target. To build
one variant deterministically:

```bash
scripts/build-board.sh waveshare-13-e6
```

This runs `idf.py` with `SDKCONFIG_DEFAULTS=sdkconfig.defaults;boards/<key>.defaults`,
puts intermediate artefacts in `build/<board-key>/`, and copies the final
binary to `dist/prostokont-<board-key>-<version>.bin`. Same command in CI; same
command locally.

Available board keys live in `boards/manifest.json`. The current set:

- `waveshare-13-e6` (ESP32-S3)
- `inkplate-2`, `inkplate-4`, `inkplate-5`, `inkplate-6`, `inkplate-6-color`, `inkplate-10` (ESP32)
- `inkplate-6-flick`, `inkplate-13` (ESP32-S3)

### Component layout

The component build keeps one shared source set and selects only the active
panel driver profile. The selected panel implementation is routed through
`components/include/panels/PanelFactory.hpp`, while compile-time metadata is
split into:

- `components/include/panels/PanelDescriptor.hpp`
- `components/include/services/board/BoardDescriptor.hpp`
- `components/include/config/FirmwareDescriptor.hpp`
- `components/include/services/board/BoardProfile.hpp`

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
4. starts the BLE provisioning service
5. tries to connect to the saved Wi-Fi network
6. falls back to setup mode if Wi-Fi is missing or unavailable

BLE provisioning is available on every boot. Setup mode additionally exposes:

- SoftAP with an SSID in the form `Prostokont-<SHORT_ID>`
- an onboarding screen with a QR code for `http://192.168.4.1`

BLE services:

- Wi-Fi provisioning service: `ask-wifi-v1`
- display service: `ble-display-v1`

The BLE provisioning and display status payloads mirror the same device,
display, panel, board, and firmware contract exposed by the local HTTP status
endpoints. Transport-specific fields such as `api`, `protocol`, and upload
progress are added on top of that shared payload shape.

BLE provisioning uses pairing and stores bond data in NVS. If a phone or tablet
has an old pairing entry from a previous firmware build, remove that saved BLE
device before pairing again.

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

The HTTP discovery payload, the HTTP status payload, and the BLE provisioning
and display status payloads now share one core contract. The shared fields are:

- `device_id`
- `name`
- `hostname`
- `model`
- `board`
- `firmware_version`
- `wifi_provisioned`
- `sta_connected`
- `display_width`
- `display_height`
- `packed_frame_supported`
- `color_scheme`
- `panel_info`
- `board_info`
- `firmware_info`

Transport-specific fields stay transport-specific:

- discovery and BLE provisioning status add `api`
- discovery adds `capabilities`
- BLE statuses add `state`
- BLE display status also adds `protocol`, `bytes_expected`,
  `bytes_received`, and `format`

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

## Releases

Per-board releases are driven by Git tags of the form `<board-key>-v<semver>`,
e.g. `waveshare-13-e6-v0.2.1`. Push such a tag and the `release` GitHub
Actions workflow builds that one variant and publishes a GitHub Release with
the `.bin` attached. Other boards remain on their own latest tags.

A hotfix for a single panel touches only that panel's tag; the other boards
stay on whatever they shipped last.

## Development Notes

- Do not commit `build/` or `dist/` artifacts.
- Board selection is controlled by `menuconfig`; `components/CMakeLists.txt`
  only maps that selection to the active driver profile.
- The generic display wrapper in `components/src/Display.cpp` no longer needs
  to import a concrete panel directly; it consumes
  `components/include/panels/PanelFactory.hpp`.
- Panel metadata lives in `components/include/panels/PanelDescriptor.hpp`.
- Board metadata lives in `components/include/services/board/BoardDescriptor.hpp`.
- Firmware metadata lives in `components/include/config/FirmwareDescriptor.hpp`.
- Service-layer display capabilities and the currently selected descriptors are
  exposed through
  `components/include/services/board/BoardProfile.hpp`.
- The main application entry point is `main/main.cpp`.
- Device state is persisted under the NVS namespace `prostokont`.

## License

This repository is licensed under the terms of the `GPL-3.0-only` license. See [LICENSE](LICENSE).
