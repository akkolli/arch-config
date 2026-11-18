-- Hyprland configuration migrated from hyprland.conf for Hyprland 0.56+.
-- Any legacy .conf files present beside it are rollback-only.

local terminal = "ghostty"
local fileManager = "dolphin"
local menu = "rofi -show drun"
local browser = "brave --password-store=basic"
local mainMod = "SUPER"

-- Monitor
hl.monitor({
    output = "",
    mode = "2560x1440@240",
    position = "auto",
    scale = "auto",
})

-- Start and stop the desktop services at the same lifecycle points as the
-- former exec-once and exec-shutdown entries.
hl.on("hyprland.start", function()
    hl.exec_cmd("/home/akkolli/.config/hypr/scripts/start-desktop-services")
end)

hl.on("hyprland.shutdown", function()
    hl.exec_cmd("systemctl --user stop graphical-session.target")
end)

-- Environment
hl.env("XCURSOR_THEME", "capitaine-cursors")
hl.env("XCURSOR_SIZE", "24")
hl.env("XDG_CURRENT_DESKTOP", "Hyprland")
hl.env("XDG_SESSION_TYPE", "wayland")
hl.env("QT_QPA_PLATFORM", "wayland;xcb")
hl.env("QT_QPA_PLATFORMTHEME", "qt6ct")
hl.env("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1")
hl.env("HYPRCURSOR_SIZE", "24")

-- Base appearance and input settings.
hl.config({
    general = {
        gaps_in = 5,
        gaps_out = 20,
        border_size = 2,
        col = {
            active_border = {
                colors = {"rgba(33ccffee)", "rgba(00ff99ee)"},
                angle = 45,
            },
            inactive_border = "rgba(595959aa)",
        },
        resize_on_border = false,
        allow_tearing = false,
        layout = "dwindle",
    },
    decoration = {
        rounding = 10,
        rounding_power = 2,
        active_opacity = 1.0,
        inactive_opacity = 1.0,
        shadow = {
            enabled = true,
            range = 4,
            render_power = 3,
            color = 0xee1a1a1a,
        },
        blur = {
            enabled = true,
            size = 3,
            passes = 1,
            vibrancy = 0.1696,
        },
    },
    animations = {
        enabled = true,
    },
    dwindle = {
        preserve_split = true,
    },
    master = {
        new_status = "master",
    },
    misc = {
        force_default_wallpaper = -1,
        disable_hyprland_logo = false,
    },
    input = {
        kb_layout = "us",
        kb_variant = "",
        kb_model = "",
        kb_options = "",
        kb_rules = "",
        follow_mouse = 1,
        sensitivity = 0,
        touchpad = {
            natural_scroll = false,
        },
    },
})

-- Curves and animations.
hl.curve("easeOutQuint", {
    type = "bezier",
    points = {{0.23, 1}, {0.32, 1}},
})
hl.curve("easeInOutCubic", {
    type = "bezier",
    points = {{0.65, 0.05}, {0.36, 1}},
})
hl.curve("linear", {
    type = "bezier",
    points = {{0, 0}, {1, 1}},
})
hl.curve("almostLinear", {
    type = "bezier",
    points = {{0.5, 0.5}, {0.75, 1}},
})
hl.curve("quick", {
    type = "bezier",
    points = {{0.15, 0}, {0.1, 1}},
})

hl.animation({leaf = "global", enabled = true, speed = 10, bezier = "default"})
hl.animation({leaf = "border", enabled = true, speed = 5.39, bezier = "easeOutQuint"})
hl.animation({leaf = "windows", enabled = true, speed = 4.79, bezier = "easeOutQuint"})
hl.animation({leaf = "windowsIn", enabled = true, speed = 4.1, bezier = "easeOutQuint", style = "popin 87%"})
hl.animation({leaf = "windowsOut", enabled = true, speed = 1.49, bezier = "linear", style = "popin 87%"})
hl.animation({leaf = "fadeIn", enabled = true, speed = 1.73, bezier = "almostLinear"})
hl.animation({leaf = "fadeOut", enabled = true, speed = 1.46, bezier = "almostLinear"})
hl.animation({leaf = "fade", enabled = true, speed = 3.03, bezier = "quick"})
hl.animation({leaf = "layers", enabled = true, speed = 3.81, bezier = "easeOutQuint"})
hl.animation({leaf = "layersIn", enabled = true, speed = 4, bezier = "easeOutQuint", style = "fade"})
hl.animation({leaf = "layersOut", enabled = true, speed = 1.5, bezier = "linear", style = "fade"})
hl.animation({leaf = "fadeLayersIn", enabled = true, speed = 1.79, bezier = "almostLinear"})
hl.animation({leaf = "fadeLayersOut", enabled = true, speed = 1.39, bezier = "almostLinear"})
hl.animation({leaf = "workspaces", enabled = true, speed = 1.94, bezier = "almostLinear", style = "fade"})
hl.animation({leaf = "workspacesIn", enabled = true, speed = 1.21, bezier = "almostLinear", style = "fade"})
hl.animation({leaf = "workspacesOut", enabled = true, speed = 1.94, bezier = "almostLinear", style = "fade"})
hl.animation({leaf = "zoomFactor", enabled = true, speed = 7, bezier = "quick"})

hl.gesture({
    fingers = 3,
    direction = "horizontal",
    action = "workspace",
})

-- This placeholder device entry was present in the generated config.
hl.device({
    name = "epic-mouse-v1",
    sensitivity = -0.5,
})

-- Window rules.
hl.window_rule({
    name = "oled-screensaver",
    match = {class = "local.oledscreensaver"},
    float = true,
    pin = true,
    move = "0 0",
    size = "monitor_w monitor_h",
    decorate = false,
    border_size = 0,
    rounding = 0,
    no_anim = true,
    no_shadow = true,
    no_blur = true,
})

hl.window_rule({
    name = "suppress-maximize-events",
    match = {class = ".*"},
    suppress_event = "maximize",
})

hl.window_rule({
    name = "fix-xwayland-drags",
    match = {
        class = "^$",
        title = "^$",
        xwayland = true,
        float = true,
        fullscreen = false,
        pin = false,
    },
    no_focus = true,
})

hl.window_rule({
    name = "move-hyprland-run",
    match = {class = "hyprland-run"},
    move = "20 monitor_h-120",
    float = true,
})

hl.window_rule({
    name = "file-dialogs",
    match = {
        title = ".*(Save File|Open File|Save As|File Upload|Select File).*",
    },
    float = true,
    center = true,
    size = "900 650",
})

hl.window_rule({
    name = "modal-popups",
    match = {modal = true},
    float = true,
    center = true,
})

-- Theme overrides are generated by ~/.local/bin/theme-switch.
require("theme")

-- Keybindings.
hl.bind(mainMod .. " + T", hl.dsp.exec_cmd(terminal))
hl.bind(mainMod .. " + W", hl.dsp.window.close())
hl.bind(mainMod .. " + E", hl.dsp.exec_cmd(fileManager))
hl.bind(mainMod .. " + C", hl.dsp.exec_cmd("~/.config/hypr/scripts/smart-clipboard C"))
hl.bind(mainMod .. " + V", hl.dsp.exec_cmd("~/.config/hypr/scripts/smart-clipboard V"))
hl.bind(mainMod .. " + R", hl.dsp.exec_cmd(menu))
hl.bind(mainMod .. " + P", hl.dsp.window.pseudo())
hl.bind(mainMod .. " + CTRL + J", hl.dsp.layout("togglesplit"))
hl.bind(mainMod .. " + SHIFT + V", hl.dsp.exec_cmd("cliphist list | rofi -dmenu | cliphist decode | wl-copy"))
hl.bind(mainMod .. " + CTRL + V", hl.dsp.window.float({action = "toggle"}))
hl.bind(mainMod .. " + SHIFT + S", hl.dsp.exec_cmd('grim -g "$(slurp)" - | swappy -f -'))
hl.bind("Print", hl.dsp.exec_cmd("grim ~/Pictures/screenshot-$(date +%F-%H-%M-%S).png"))
hl.bind(mainMod .. " + B", hl.dsp.exec_cmd(browser))
hl.bind(mainMod .. " + D", hl.dsp.exec_cmd(browser .. " --app=https://discord.com/channels/@me"))
hl.bind(mainMod .. " + SHIFT + Q", hl.dsp.exec_cmd("$HOME/.config/waybar/scripts/power-menu"))
hl.bind(mainMod .. " + G", hl.dsp.exec_cmd(browser .. " --app=https://gmail.com"))
hl.bind(mainMod .. " + M", hl.dsp.exec_cmd(browser .. " --app=https://music.youtube.com"))
hl.bind(mainMod .. " + A", hl.dsp.exec_cmd(browser .. " --app=https://chatgpt.com"))
hl.bind(mainMod .. " + SHIFT + R", hl.dsp.exec_cmd("/home/akkolli/.local/bin/theme-menu"))

hl.bind(mainMod .. " + H", hl.dsp.focus({direction = "l"}))
hl.bind(mainMod .. " + L", hl.dsp.focus({direction = "r"}))
hl.bind(mainMod .. " + K", hl.dsp.focus({direction = "u"}))
hl.bind(mainMod .. " + J", hl.dsp.focus({direction = "d"}))

hl.bind(mainMod .. " + SHIFT + H", hl.dsp.window.move({direction = "l"}))
hl.bind(mainMod .. " + SHIFT + L", hl.dsp.window.move({direction = "r"}))
hl.bind(mainMod .. " + SHIFT + K", hl.dsp.window.move({direction = "u"}))
hl.bind(mainMod .. " + SHIFT + J", hl.dsp.window.move({direction = "d"}))

hl.bind(mainMod .. " + CTRL + left", hl.dsp.window.resize({x = -40, y = 0, relative = true}), {repeating = true})
hl.bind(mainMod .. " + CTRL + right", hl.dsp.window.resize({x = 40, y = 0, relative = true}), {repeating = true})
hl.bind(mainMod .. " + CTRL + up", hl.dsp.window.resize({x = 0, y = -40, relative = true}), {repeating = true})
hl.bind(mainMod .. " + CTRL + down", hl.dsp.window.resize({x = 0, y = 40, relative = true}), {repeating = true})

for i = 1, 10 do
    local key = i % 10
    hl.bind(mainMod .. " + " .. key, hl.dsp.focus({workspace = i}))
    hl.bind(mainMod .. " + SHIFT + " .. key, hl.dsp.window.move({workspace = i}))
end

hl.bind(mainMod .. " + mouse_down", hl.dsp.focus({workspace = "e+1"}))
hl.bind(mainMod .. " + mouse_up", hl.dsp.focus({workspace = "e-1"}))
hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), {mouse = true})
hl.bind(mainMod .. " + mouse:273", hl.dsp.window.resize(), {mouse = true})

hl.bind("XF86AudioRaiseVolume", hl.dsp.exec_cmd("wpctl set-volume -l 1 @DEFAULT_AUDIO_SINK@ 5%+"), {locked = true, repeating = true})
hl.bind("XF86AudioLowerVolume", hl.dsp.exec_cmd("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-"), {locked = true, repeating = true})
hl.bind("XF86AudioMute", hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle"), {locked = true, repeating = true})
hl.bind("XF86AudioMicMute", hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle"), {locked = true, repeating = true})
hl.bind("XF86MonBrightnessUp", hl.dsp.exec_cmd("sh -c 'command -v brightnessctl >/dev/null && brightnessctl -e4 -n2 set 5%+'"), {locked = true, repeating = true})
hl.bind("XF86MonBrightnessDown", hl.dsp.exec_cmd("sh -c 'command -v brightnessctl >/dev/null && brightnessctl -e4 -n2 set 5%-'"), {locked = true, repeating = true})

hl.bind("XF86AudioNext", hl.dsp.exec_cmd("playerctl next"), {locked = true})
hl.bind("XF86AudioPause", hl.dsp.exec_cmd("playerctl play-pause"), {locked = true})
hl.bind("XF86AudioPlay", hl.dsp.exec_cmd("playerctl play-pause"), {locked = true})
hl.bind("XF86AudioPrev", hl.dsp.exec_cmd("playerctl previous"), {locked = true})
