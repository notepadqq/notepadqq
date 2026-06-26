#!/bin/bash
set -euo pipefail

build_dir="${1:-out/release}"
dist_dir="${2:-dist}"
appdir="${3:-AppDir}"
artifact_name="${4:-notepadqq-nightly-linux-x86_64}"

mkdir -p "$appdir/usr" "$dist_dir"

if [[ ! -x ./linuxdeploy.AppImage ]]; then
    curl -L --retry 3 -o linuxdeploy.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy.AppImage
fi

if [[ ! -x ./linuxdeploy-plugin-qt.AppImage ]]; then
    curl -L --retry 3 -o linuxdeploy-plugin-qt.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt.AppImage
fi

cmake --install "$build_dir" --prefix "$PWD/$appdir/usr"
./linuxdeploy.AppImage --appdir "$appdir" --plugin qt --output appimage
mv ./*.AppImage "$dist_dir/${artifact_name}.AppImage"
