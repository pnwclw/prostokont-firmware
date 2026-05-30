#!/usr/bin/env bash
# Build a single (board, panel) target.
#
# Usage:
#   scripts/build-board.sh <board-key>
#
# Reads boards/manifest.json to look up the IDF target for the given board key,
# then runs idf.py with the base sdkconfig.defaults plus boards/<board-key>.defaults
# in an isolated build directory under build/<board-key>/.
#
# The resulting binary is copied to dist/prostokont-<board-key>-<version>.bin,
# where <version> is read from components/include/config/AppConfig.hpp.

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <board-key>" >&2
  exit 2
fi

BOARD="$1"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MANIFEST="$ROOT/boards/manifest.json"
OVERLAY="$ROOT/boards/$BOARD.defaults"
BUILD_DIR="$ROOT/build/$BOARD"
DIST_DIR="$ROOT/dist"

if [[ ! -f "$MANIFEST" ]]; then
  echo "Missing manifest: $MANIFEST" >&2
  exit 1
fi

if [[ ! -f "$OVERLAY" ]]; then
  echo "Missing overlay for board '$BOARD': $OVERLAY" >&2
  exit 1
fi

TARGET="$(jq -r --arg key "$BOARD" '.boards[] | select(.key==$key) | .target' "$MANIFEST")"
if [[ -z "$TARGET" || "$TARGET" == "null" ]]; then
  echo "Board '$BOARD' not found in $MANIFEST" >&2
  exit 1
fi

VERSION="$(grep -E 'kFirmwareVersion[[:space:]]*=' \
  "$ROOT/components/include/config/AppConfig.hpp" \
  | head -n1 | sed -E 's/.*"([^"]+)".*/\1/')"

if [[ -z "$VERSION" ]]; then
  echo "Could not read kFirmwareVersion from AppConfig.hpp" >&2
  exit 1
fi

echo "==> Building $BOARD (target=$TARGET, version=$VERSION)"

mkdir -p "$DIST_DIR"

export IDF_TARGET="$TARGET"
export SDKCONFIG_DEFAULTS="$ROOT/sdkconfig.defaults;$OVERLAY"

(
  cd "$ROOT"
  idf.py -B "$BUILD_DIR" set-target "$TARGET"
  idf.py -B "$BUILD_DIR" build
)

ARTIFACT="$BUILD_DIR/prostokont-firmware.bin"
if [[ ! -f "$ARTIFACT" ]]; then
  echo "Expected artifact missing: $ARTIFACT" >&2
  exit 1
fi

OUT="$DIST_DIR/prostokont-$BOARD-$VERSION.bin"
cp "$ARTIFACT" "$OUT"
echo "==> $OUT"
