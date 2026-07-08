#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cp -a "$repo_dir/home/." "$HOME/"

chmod +x \
  "$HOME/.config/hypr/scripts/smart-clipboard" \
  "$HOME/.config/hypr/scripts/start-desktop-services" \
  "$HOME/.config/waybar/scripts/audio-device-menu" \
  "$HOME/.config/waybar/scripts/gpu" \
  "$HOME/.config/waybar/scripts/power-menu" \
  "$HOME/.config/waybar/scripts/ram" \
  "$HOME/.config/waybar/scripts/weather" \
  "$HOME/.local/bin/theme-menu" \
  "$HOME/.local/bin/theme-switch" \
  "$HOME/.local/share/quickshell-lockscreen/lock.sh"

if command -v systemctl >/dev/null 2>&1; then
  systemctl --user daemon-reload || true
  systemctl --user enable \
    hyprpaper.service \
    hyprpolkitagent.service \
    mako.service \
    hypridle.service \
    cliphist.service \
    waybar-hyprland.service \
    no-idle-sleep.service || true
fi

printf 'Installed desktop config overlay into %s\n' "$HOME"
printf 'Run ~/.local/bin/theme-switch anime to regenerate active theme files for this user.\n'
