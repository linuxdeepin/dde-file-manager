// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mywindowplugin.h"

#include <iostream>

namespace Exapmle {

MyWindowPlugin::MyWindowPlugin()
{
    registerWindowOpened([this](uint64_t winId) {
        windowOpened(winId);
    });
    registerWindowClosed([this](uint64_t winId) {
        windowClosed(winId);
    });
    registerFirstWindowOpened([this](uint64_t winId) {
        firstWindowOpened(winId);
    });
    registerLastWindowClosed([this](uint64_t winId) {
        lastWindowClosed(winId);
    });
    registerWindowUrlChanged([this](uint64_t winId, const std::string &urlString) {
        windowUrlChanged(winId, urlString);
    });
}

void MyWindowPlugin::windowOpened(uint64_t winId)
{
    std::cerr << "-->Ext:" << __PRETTY_FUNCTION__ << winId << std::endl;
}

void MyWindowPlugin::windowClosed(uint64_t winId)
{
    std::cerr << "-->Ext:" << __PRETTY_FUNCTION__ << winId << std::endl;
}

void MyWindowPlugin::firstWindowOpened(uint64_t winId)
{
    std::cerr << "-->Ext:" << __PRETTY_FUNCTION__ << winId << std::endl;
}

void MyWindowPlugin::lastWindowClosed(uint64_t winId)
{
    std::cerr << "-->Ext:" << __PRETTY_FUNCTION__ << winId << std::endl;
}

void MyWindowPlugin::windowUrlChanged(uint64_t winId, const std::string &urlString)
{
    std::cerr << "-->Ext:" << __PRETTY_FUNCTION__ << winId << urlString << std::endl;
}

}   // namespace Exapmle
