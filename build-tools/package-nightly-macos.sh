#!/bin/bash
set -euo pipefail

build_dir="${1:-build/release}"
dist_dir="${2:-dist}"
staging_dir="${3:-nightly-root}"
artifact_name="${4:-notepadqq-nightly-macos-x86_64}"
qt_prefix="${QT_PREFIX:-$(brew --prefix qt@6)}"

mkdir -p "$dist_dir"

bundle="$PWD/$staging_dir/notepadqq.app"
qt_bin="$qt_prefix/bin"

cmake --install "$build_dir" --prefix "$PWD/$staging_dir"

"$qt_bin/macdeployqt" "$bundle" -executable="$bundle/Contents/MacOS/notepadqq"
shopt -s nullglob
helpers=("$bundle"/Contents/Frameworks/QtWebEngineCore.framework/Versions/*/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess)
if [[ -n "${helpers[0]:-}" ]]; then
    "$qt_bin/macdeployqt" "$bundle" -executable="${helpers[0]}"
fi

ditto -c -k --sequesterRsrc --keepParent "$bundle" "$dist_dir/${artifact_name}.zip"
