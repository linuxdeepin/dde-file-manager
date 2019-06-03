QT       += core widgets

TARGET = lff

SOURCES += main.cpp

BINDIR = $$PREFIX/bin

target.path = $$BINDIR

autostart.path = /etc/xdg/autostart
autostart.files = lff-autostart.desktop

INSTALLS += autostart
