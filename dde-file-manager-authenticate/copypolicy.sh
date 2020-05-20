#!/bin/sh
sudo chmod +x deepin-vault-authenticateProxy;
sudo cp deepin-vault-authenticateProxy /usr/bin/;
sudo cp policy/deepin-vault-authenticateProxy.policy /usr/share/polkit-1/actions/;
