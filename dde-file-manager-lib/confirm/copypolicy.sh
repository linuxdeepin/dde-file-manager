#!/bin/sh
sudo chmod +x testVault;
sudo cp testVault /usr/bin/;
sudo cp policy/testVault.policy /usr/share/polkit-1/actions/;

sudo chmod +x deepin-vaultRemove-authenticateProxy;
sudo cp deepin-vaultRemove-authenticateProxy /usr/bin/;
sudo cp policy/com.deepin.pkexec.deepin-vaultRemove-authenticateProxy.policy /usr/share/polkit-1/actions/;
