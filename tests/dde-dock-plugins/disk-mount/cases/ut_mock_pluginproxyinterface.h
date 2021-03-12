/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gmock/gmock.h>

#include "pluginproxyinterface.h"

class MockPluginProxyInterface:public PluginProxyInterface{

public:
    MOCK_METHOD2(itemAdded, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD2(itemUpdate, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD2(itemRemoved, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD3(requestWindowAutoHide, void(PluginsItemInterface * const, const QString &, const bool));
    MOCK_METHOD2(requestRefreshWindowVisible, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD3(requestSetAppletVisible, void(PluginsItemInterface * const, const QString &, const bool));
    MOCK_METHOD3(saveValue, void(PluginsItemInterface * const, const QString &, const QVariant &));
    MOCK_METHOD3(getValue, const QVariant(PluginsItemInterface * const, const QString &, const QVariant&));
    MOCK_METHOD2(removeValue, void(PluginsItemInterface *const, const QStringList &));
};
