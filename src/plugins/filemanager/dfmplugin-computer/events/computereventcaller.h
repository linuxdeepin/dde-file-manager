/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef COMPUTEREVENTCALLER_H
#define COMPUTEREVENTCALLER_H

#include "dfmplugin_computer_global.h"
#include "dfm-base/file/entry/entryfileinfo.h"

#include <QUrl>

class QWidget;
DPCOMPUTER_BEGIN_NAMESPACE

class ComputerEventCaller
{
public:
    ComputerEventCaller() = delete;

    static void cdTo(QWidget *sender, const QUrl &url);
    static void cdTo(QWidget *sender, const QString &path);
    static void cdTo(quint64 winId, const QUrl &url);
    static void cdTo(quint64 winId, const QString &path);

    static void sendEnterInNewWindow(const QUrl &url);
    static void sendEnterInNewTab(quint64 winId, const QUrl &url);

    static void sendContextActionTriggered(quint64 winId, const QUrl &url, const QString &action);
    static void sendOpenItem(quint64 winId, const QUrl &url);
    static void sendShowFilePropertyDialog(const QUrl &url);
    static void sendShowDevicePropertyDialog(const DFMEntryFileInfoPointer &info);
    static void sendErase(const QString &dev);
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTEREVENTCALLER_H
