# Arch Config

Arch Linux desktop configuration overlay.

This repo stores the live Hyprland setup as a `$HOME` overlay under `home/`.
It includes Hyprland, Hyprpaper, Hypridle, Hyprlock, Waybar, Rofi, Mako,
Ghostty, GTK/Qt theme settings, wallpapers, theme switching scripts, user
systemd units, and the Quickshell lockscreen theme used by the power menu.

## Install

Install the desktop packages first:

```sh
xargs -a packages/desktop-packages.txt yay -S --needed
```

Then copy the config overlay into your home directory and enable the user
services:

```sh
./install.sh
```

Start Hyprland normally. On login, Hyprland runs
`~/.config/hypr/scripts/start-desktop-services`, which starts Hyprpaper, Mako,
Hypridle, Cliphist, Hyprpolkitagent, and Waybar through systemd user services.

## Notes

- Keyboard shortcuts live in `home/.config/hypr/hyprland.conf`.
- Wallpapers live in `home/.config/themes/wallpapers`.
- `Super+Shift+R` opens the theme menu.
- `~/.local/bin/theme-switch anime|forest|mountain|ocean` regenerates the
  active theme files and applies the matching wallpaper.
- Waybar weather reads `WAYBAR_WEATHER_LOCATION` or an untracked local
  `~/.config/waybar/weather-location` file.
