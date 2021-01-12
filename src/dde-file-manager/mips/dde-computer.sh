#!/bin/bash

process=`ps ax -o 'cmd' |grep 'dde-file-manager$'`;
processd=`ps ax -o 'cmd' |grep 'dde-file-manager -d$'`;
  
if [[ "$process" == ""&&"$processd" == "" ]]; then
    dde-file-manager computer:///
else
    file-manager.sh  computer:///
fi
