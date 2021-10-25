#!/bin/bash


echo "=====vault====="
#qdbuscpp2xml -M -S ../vault/vaultmanager.h -o vault.xml
qdbusxml2cpp -i ../vault/vaultmanager.h -c VaultAdaptor -l VaultManager -a ../dbusservice/dbusadaptor/vault_adaptor vault.xml
qdbusxml2cpp -c VaultInterface -p ../dbusservice/dbusinterface/vault_interface vault.xml


#qdbuscpp2xml -M -S ../vault/vaultbruteforceprevention.h -o vaultbruteforceprevention.xml
qdbusxml2cpp -i ../vault/vaultbruteforceprevention.h -c VaultBruteForcePreventionAdaptor -l VaultBruteForcePrevention -a ../dbusservice/dbusadaptor/vaultbruteforceprevention_adaptor vaultbruteforceprevention.xml
qdbusxml2cpp -c VaultBruteForcePreventionInterface -p ../dbusservice/dbusinterface/vaultbruteforceprevention_interface vaultbruteforceprevention.xml

