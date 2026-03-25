#!/bin/zsh
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <path-to-app-bundle> [path-to-macdeployqt]" >&2
    exit 1
fi

APP_PATH="$1"
MACDEPLOYQT="${2:-macdeployqt}"

APP_PATH="$(cd "$(dirname "$APP_PATH")" && pwd)/$(basename "$APP_PATH")"
MACOS_DIR="$APP_PATH/Contents/MacOS"
LAUNCHER_PATH="$MACOS_DIR/CrochetCharts"
REAL_BINARY_PATH="$MACOS_DIR/CrochetCharts-bin"
TEMP_SCRIPT_PATH="$MACOS_DIR/.CrochetCharts-launcher-tmp"

if [[ ! -d "$APP_PATH" ]]; then
    echo "App bundle not found: $APP_PATH" >&2
    exit 1
fi

if [[ ! -d "$MACOS_DIR" ]]; then
    echo "MacOS directory not found in bundle: $MACOS_DIR" >&2
    exit 1
fi

if [[ -f "$REAL_BINARY_PATH" ]]; then
    if [[ -f "$LAUNCHER_PATH" ]]; then
        mv "$LAUNCHER_PATH" "$TEMP_SCRIPT_PATH"
    fi
    mv "$REAL_BINARY_PATH" "$LAUNCHER_PATH"
fi

"$MACDEPLOYQT" "$APP_PATH"

mv "$LAUNCHER_PATH" "$REAL_BINARY_PATH"

cat > "$LAUNCHER_PATH" <<'EOF'
#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

unset DYLD_FRAMEWORK_PATH
unset DYLD_LIBRARY_PATH
unset DYLD_INSERT_LIBRARIES
unset DYLD_FALLBACK_FRAMEWORK_PATH
unset DYLD_FALLBACK_LIBRARY_PATH
unset QT_PLUGIN_PATH
unset QT_QPA_PLATFORM_PLUGIN_PATH

export QT_PLUGIN_PATH="$SCRIPT_DIR/../PlugIns"
export QT_QPA_PLATFORM_PLUGIN_PATH="$SCRIPT_DIR/../PlugIns/platforms"

exec "$SCRIPT_DIR/CrochetCharts-bin" "$@"
EOF

chmod +x "$LAUNCHER_PATH"
rm -f "$TEMP_SCRIPT_PATH"
