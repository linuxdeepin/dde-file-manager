#!/bin/bash

echo "-->make XML of TagDBus"
qdbuscpp2xml -M -S tagdbus.h -o ./org.deepin.filemanager.tag.xml
