#!/bin/bash

process=`ps ax -o 'cmd' |grep 'dde-file-manager$'`;


if [ "$process" == "" ]; then
    if [[ $# -ge 1 ]]; then
        dde-file-manager "$@"
    else
        dde-file-manager $HOME
    fi
else
    if [[ $# -ge 1 ]]; then
        if [[ "$1" == "--new-window" || "$1" == "-n" ]]; then
            _paths=""
            for item in "$@"; do
                if [[ "$item" == "$1" ]]; then
                    continue
                fi
                _paths=$_paths\"$item\"","
            done
            let i=${#_paths}-1
            _paths=${_paths:0:$i}
            _paths="["$_paths"]"
            echo "{\"paths\":$paths, \"isNewWindow\": true}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
        else
            paths=""
            for item in "$@"; do
                paths=$paths\"$item\"","
            done
            let i=${#paths}-1
            paths=${paths:0:$i}
            paths="["$paths"]"
            echo "{\"paths\":$paths}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
        fi
    else
        echo "{\"paths\":[\"\~\"]}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
    fi
fi