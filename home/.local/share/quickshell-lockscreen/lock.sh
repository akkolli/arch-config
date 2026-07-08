#!/usr/bin/env bash

# Current directory
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library paths
export QML2_IMPORT_PATH="$DIR/imports:$QML2_IMPORT_PATH"
export QML_XHR_ALLOW_FILE_READ=1

# Get session type
export XDG_SESSION_TYPE="${XDG_SESSION_TYPE:-$(loginctl show-session $(loginctl | grep $(whoami) | awk '{print $1}') -p Type --value 2>/dev/null || echo wayland)}"

CONFIG_FILE="$HOME/.config/qylock/theme"
if [ -n "$1" ]; then
    export QS_THEME="$1"
elif [ -f "$CONFIG_FILE" ]; then
    export QS_THEME="$(tr -d '\r\n' < "$CONFIG_FILE")"
    export QS_THEME="${QS_THEME:-winter}"
else
    export QS_THEME="winter"
fi

# Set theme path
if [ -d "$DIR/themes/$QS_THEME" ]; then
    export QS_THEME_PATH="$DIR/themes/$QS_THEME"
elif [ -d "$DIR/../themes/$QS_THEME" ]; then
    export QS_THEME_PATH="$DIR/../themes/$QS_THEME"
else
    echo "Missing Quickshell lockscreen theme: $QS_THEME" >&2
    exit 1
fi

echo "Locking with Quickshell using theme: $QS_THEME"
echo "Theme path: $QS_THEME_PATH"

# Kill active lockers
killall -9 hyprlock swaylock wlogout 2>/dev/null || true

# Execute lock screen
quickshell -p "$DIR/lock_shell.qml"
