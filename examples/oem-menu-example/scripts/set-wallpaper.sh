#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# set-wallpaper.sh — Set an image file as the desktop background.
#
# Usage: set-wallpaper.sh <image-path>
# Supports Deepin/UOS (org.deepin.dde.Appearance1.SetMonitorBackground via D-Bus)
# and standard GNOME (org.gnome.desktop.background).

set -euo pipefail

IMAGE_PATH="${1:-}"

if [ -z "$IMAGE_PATH" ]; then
    echo "Error: no image path specified" >&2
    exit 1
fi

ABS_IMAGE="$(realpath -- "$IMAGE_PATH")"

if [ ! -f "$ABS_IMAGE" ]; then
    echo "Error: file not found: $ABS_IMAGE" >&2
    exit 1
fi

notify() {
    notify-send --app-name="OEM菜单" "$1" "$2" 2>/dev/null || true
}

IMAGE_URI="file://${ABS_IMAGE}"

set_wallpaper_deepin() {
    # Deepin/UOS: use org.deepin.dde.Appearance1.SetMonitorBackground via D-Bus.
    # Monitor name is taken from the first connected output reported by xrandr.
    if ! command -v dbus-send &>/dev/null; then
        return 1
    fi
    local monitor
    monitor="$(xrandr 2>/dev/null | grep ' connected' | head -n1 | awk '{print $1}')"
    if [ -z "$monitor" ]; then
        return 1
    fi
    dbus-send --session --print-reply=literal \
        --dest=org.deepin.dde.Appearance1 \
        /org/deepin/dde/Appearance1 \
        org.deepin.dde.Appearance1.SetMonitorBackground \
        string:"$monitor" \
        string:"$ABS_IMAGE" &>/dev/null
}

set_wallpaper_gnome() {
    # Standard GNOME gsettings schema
    local schema="org.gnome.desktop.background"
    if gsettings get "$schema" picture-uri &>/dev/null 2>&1; then
        gsettings set "$schema" picture-uri "$IMAGE_URI"
        # Also set dark mode variant if schema supports it
        gsettings set "$schema" picture-uri-dark "$IMAGE_URI" 2>/dev/null || true
        return 0
    fi
    return 1
}

set_wallpaper_xfconf() {
    # XFCE fallback via xfconf-query
    if command -v xfconf-query &>/dev/null; then
        local monitors
        monitors="$(xfconf-query -c xfce4-desktop -l 2>/dev/null | grep '/backdrop/screen0' | grep '/last-image' || true)"
        if [ -n "$monitors" ]; then
            while IFS= read -r prop; do
                xfconf-query -c xfce4-desktop -p "$prop" -s "$ABS_IMAGE" 2>/dev/null || true
            done <<< "$monitors"
            return 0
        fi
    fi
    return 1
}

if set_wallpaper_deepin; then
    notify "壁纸已设置" "$(basename "$ABS_IMAGE")"
elif set_wallpaper_gnome; then
    notify "壁纸已设置" "$(basename "$ABS_IMAGE")"
elif set_wallpaper_xfconf; then
    notify "壁纸已设置" "$(basename "$ABS_IMAGE")"
else
    notify "设置壁纸失败" "当前桌面环境不支持自动设置壁纸"
    exit 1
fi
