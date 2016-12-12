#!/bin/bash

process=`ps ax -o 'cmd' |grep 'dde-file-manager$'`;

if [ "$process" == "" ]; then
	if [[ $# -ge 1 ]]; then
		dde-file-manager $1 $2
	else
		dde-file-manager $HOME
	fi
else
	if [[ $# -ge 1 ]]; then
		if [[ "$1" == "--new-window" || "$1" == "-n" ]]; then
			echo "{\"url\":\"\~\", \"isNewWindow\": true}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
		else
			echo "{\"url\":\"$1\"}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
		fi
	else
		echo "{\"url\":\"\~\"}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
	fi
fi