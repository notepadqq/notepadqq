#!/bin/bash
set -euo pipefail

build_dir="${1:-build/release}"
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

# appimagetool currently checks legacy *.appdata.xml naming for AppStream and
# rejects our canonical CID-based metainfo filename as a mismatch.
LDAI_NO_APPSTREAM=1 ./linuxdeploy.AppImage --appdir "$appdir" --plugin qt --output appimage

# linuxdeploy and its Qt plugin are themselves AppImages in the repo root, so
# collect only the generated package instead of moving every ./*.AppImage match.
generated_appimages=()
shopt -s nullglob
for appimage in ./*.AppImage; do
    case "$(basename "$appimage")" in
        linuxdeploy*.AppImage)
            continue
            ;;
    esac
    generated_appimages+=("$appimage")
done
shopt -u nullglob

if [[ ${#generated_appimages[@]} -ne 1 ]]; then
    echo "Expected exactly one generated AppImage, found ${#generated_appimages[@]}" >&2
    printf 'Candidates: %s\n' "${generated_appimages[@]:-<none>}" >&2
    exit 1
fi

mv "${generated_appimages[0]}" "$dist_dir/${artifact_name}.AppImage"
