#!/bin/bash


echo "=====vault====="
#qdbuscpp2xml -M -S ../vault/vaultmanager.h -o vault.xml
qdbusxml2cpp -i ../vault/vaultmanager.h -c VaultAdaptor -l VaultManager -a ../dbusservice/dbusadaptor/vault_adaptor vault.xml
qdbusxml2cpp -c VaultInterface -p ../dbusservice/dbusinterface/vault_interface vault.xml

