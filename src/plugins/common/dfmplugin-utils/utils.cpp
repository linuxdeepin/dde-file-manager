/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "utils.h"
#include "global/virtualglobalplugin.h"
#include "openwith/virtualopenwithplugin.h"
#include "appendcompress/virtualappendcompressplugin.h"
#include "bluetooth/virtualbluetoothplugin.h"

using namespace dfmplugin_utils;

static constexpr char kAppendcompress[] { "dfmplugin-appendcompress" };
static constexpr char kBluetooth[] { "dfmplugin-bluetooth" };
static constexpr char kGlobal[] { "dfmplugin-global" };
static constexpr char kOpenWith[] { "dfmplugin-openwith" };

QSharedPointer<DPF_NAMESPACE::Plugin> VirtualUtils::create(const QString &pluginName)
{
    qInfo() << "Create plugin: " << pluginName;

    if (pluginName == kAppendcompress) {
        QSharedPointer<DPF_NAMESPACE::Plugin> pugin { new VirtualAppendCompressPlugin };
        return pugin;
    }

    if (pluginName == kBluetooth) {
        QSharedPointer<DPF_NAMESPACE::Plugin> pugin { new VirtualBluetoothPlugin };
        return pugin;
    }

    if (pluginName == kGlobal) {
        QSharedPointer<DPF_NAMESPACE::Plugin> pugin { new VirtualGlobalPlugin };
        return pugin;
    }

    if (pluginName == kOpenWith) {
        QSharedPointer<DPF_NAMESPACE::Plugin> pugin { new VirtualOpenWithPlugin };
        return pugin;
    }

    return {};
}
