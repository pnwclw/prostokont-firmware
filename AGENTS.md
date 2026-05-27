# AGENTS.md

Repository guidance for coding agents working in this project.

## Scope

These instructions apply to the entire repository.

## Project Summary

- This is an ESP-IDF firmware project for the Prostokont e-paper frame.
- The current checked-in target is `esp32s3`.
- The current checked-in board selection is `CONFIG_WAVESHARE_BOARD_WAVESHARE13=y`.
- The firmware includes display control, BLE and SoftAP provisioning, local HTTP APIs, mDNS, and NVS-backed storage.

## Working Rules

1. Keep all documentation, comments, commit messages, and user-facing text in English.
2. Prefer small, local changes that match the existing ESP-IDF and CMake structure.
3. Do not rename the top-level `components` module unless the user explicitly asks for a broader refactor.
4. Do not commit generated directories or files such as `build/`, IDE metadata, or temporary logs unless the user explicitly asks for them.
5. When changing API behavior, provisioning flow, storage keys, or setup instructions, update `README.md` in the same change.
6. Keep board-specific logic inside the existing panel, feature, or service layers instead of adding ad hoc logic to `main/main.cpp`.
7. Preserve compatibility with the current checked-in board configuration unless the user explicitly asks to change the target hardware.
8. Prefer ASCII in new files unless a non-ASCII character is required by the source material.

## Verification

- For firmware code changes, prefer validating with `idf.py build`.
- For documentation-only changes, a build is not required unless the user asks for it.

## Important Paths

- `main/main.cpp` - firmware bootstrap
- `components/CMakeLists.txt` - board-dependent source selection
- `components/include/config/AppConfig.hpp` - compile-time app configuration
- `components/src/services/` - provisioning, HTTP, storage, Wi-Fi, mDNS
