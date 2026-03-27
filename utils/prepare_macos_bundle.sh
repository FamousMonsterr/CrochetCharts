#!/bin/zsh
set -euo pipefail
setopt typesetsilent

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <path-to-app-bundle> [path-to-macdeployqt]" >&2
    exit 1
fi

APP_PATH="$1"
MACDEPLOYQT="${2:-macdeployqt}"

APP_PATH="$(cd "$(dirname "$APP_PATH")" && pwd)/$(basename "$APP_PATH")"
MACOS_DIR="$APP_PATH/Contents/MacOS"
FRAMEWORKS_DIR="$APP_PATH/Contents/Frameworks"
PLUGINS_DIR="$APP_PATH/Contents/PlugIns"
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

sanitize_qt_install_names() {
    local binary="$1"
    local changes=()

    while IFS= read -r line; do
        local dep_path
        dep_path="$(printf '%s\n' "$line" | sed 's/ (.*$//')"
        local replacement_path=""

        if [[ "$dep_path" == /usr/local/opt/qt@5/plugins/* || "$dep_path" == /opt/homebrew/opt/qt@5/plugins/* ]]; then
            local plugin_rel="${dep_path#*/plugins/}"
            if [[ -f "$PLUGINS_DIR/$plugin_rel" ]]; then
                replacement_path="@executable_path/../PlugIns/$plugin_rel"
            fi
        elif [[ "$dep_path" == /usr/local/Cellar/qt@5/*/lib/* || "$dep_path" == /usr/local/opt/qt@5/lib/* || "$dep_path" == /opt/homebrew/Cellar/qt@5/*/lib/* || "$dep_path" == /opt/homebrew/opt/qt@5/lib/* ]]; then
            local framework_name="$(basename "$dep_path")"
            if [[ -f "$FRAMEWORKS_DIR/$framework_name" ]]; then
                replacement_path="@executable_path/../Frameworks/$framework_name"
            elif [[ "$dep_path" == *.framework/* ]]; then
                local framework_rel="${dep_path#*/lib/}"
                if [[ -f "$FRAMEWORKS_DIR/$framework_rel" ]]; then
                    replacement_path="@executable_path/../Frameworks/$framework_rel"
                fi
            fi
        fi

        if [[ -n "$replacement_path" ]]; then
            changes+=(-change "$dep_path" "$replacement_path")
        fi
    done < <(otool -L "$binary" 2>/dev/null | tail -n +2 | sed 's/^[[:space:]]*//')

    if [[ ${#changes[@]} -gt 0 ]]; then
        install_name_tool "${changes[@]}" "$binary"
    fi

    if [[ "$binary" == "$PLUGINS_DIR/"*".dylib" ]]; then
        local plugin_rel="${binary#"$PLUGINS_DIR/"}"
        install_name_tool -id "@executable_path/../PlugIns/$plugin_rel" "$binary"
    fi
}

while IFS= read -r -d '' binary; do
    sanitize_qt_install_names "$binary"
done < <(find "$APP_PATH/Contents" -type f \( -name '*.dylib' -o -perm -111 \) -print0)

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
