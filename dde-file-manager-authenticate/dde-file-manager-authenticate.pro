QT += core
QT -= gui

CONFIG += c++11

TARGET = deepin-vault-authenticateProxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/main.cpp
DEFINES += QT_DEPRECATED_WARNINGS

target.path = /usr/bin

policy.path = /usr/share/polkit-1/actions
policy.files = policy/deepin-vault-authenticateProxy.policy

INSTALLS += target policy

DISTFILES += \
    policy/deepin-vault-authenticateProxy.policy
