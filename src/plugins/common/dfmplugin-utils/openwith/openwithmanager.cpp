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
#include "openwithmanager.h"
#include "openwith/openwithhelper.h"

#include <dfm-framework/dpf.h>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

using namespace dfmplugin_utils;

OpenWithManager &OpenWithManager::instance()
{
    static OpenWithManager ins;
    return ins;
}

void OpenWithManager::init()
{
    eventReceiver->initEventConnect();
}

void OpenWithManager::start()
{
    CustomViewExtensionView func { OpenWithHelper::createOpenWithWidget };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, 2);
}

OpenWithManager::OpenWithManager(QObject *parent)
    : QObject(parent)
{
}
