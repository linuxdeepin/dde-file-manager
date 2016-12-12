#!/bin/bash

process=`ps ax -o 'cmd' |grep 'dde-file-manager$'`;
  
if [ "$process" == "" ]; then
    dde-file-manager trash:///
else
    echo "{\"url\":\"trash:///\"}" |socat - $XDG_RUNTIME_DIR/dde-file-manager
fi