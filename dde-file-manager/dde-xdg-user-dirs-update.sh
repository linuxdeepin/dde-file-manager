#!/bin/bash

declare -A XDG_USER_DIRS_TABLE=(
    ["DESKTOP"]="Desktop"
    ["DOCUMENTS"]="Documents"
    ["DOWNLOAD"]="Downloads"
    ["MUSIC"]="Music"
    ["PICTURES"]="Pictures"
    ["VIDEOS"]="Videos"
    ["PUBLICSHARE"]=".Public"
    ["TEMPLATES"]=".Templates"
)

for item in ${!XDG_USER_DIRS_TABLE[@]}; do
    xdg_dir_old=$(xdg-user-dir $item)
    xdg_dir_new="$HOME/${XDG_USER_DIRS_TABLE[$item]}"
    case $item in
        TEMPLATES | PUBLICSHARE )
            if [[ "$xdg_dir_old" != "$HOME" ]] &&\
            [[ "$xdg_dir_old" != "$xdg_dir_new" ]] &&\
            [[ -d "$xdg_dir_old" ]]; then
                mv "$xdg_dir_old" "$xdg_dir_new"
                xdg-user-dirs-update --set $item "$xdg_dir_new"
            fi
        ;;
        * )
            if [[  "$xdg_dir_old" = "$HOME" ]]; then
                [[ -d "$xdg_dir_new" ]] || mkdir -p "$xdg_dir_new"
                xdg-user-dirs-update --set $item "$xdg_dir_new"
            fi
        ;;
    esac
done