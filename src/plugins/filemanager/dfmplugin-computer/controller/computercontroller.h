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
#ifndef COMPUTERCONTROLLER_H
#define COMPUTERCONTROLLER_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/file/entry/entryfileinfo.h"

#include <QUrl>
#include <QObject>

#define ComputerControllerInstance DPCOMPUTER_NAMESPACE::ComputerController::instance()

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerController : public QObject
{
    Q_OBJECT
public:
    static ComputerController *instance();

    enum ActionAfterMount {
        kEnterDirectory,
        kEnterInNewWindow,
        kEnterInNewTab,
        kNone,
    };

    void onOpenItem(quint64 winId, const QUrl &url);
    void onMenuRequest(quint64 winId, const QUrl &url, bool triggerFromSidebar);
    void doRename(quint64 winId, const QUrl &url, const QString &name);
    void doSetAlias(DFMEntryFileInfoPointer info, const QString &alias);

    void mountDevice(quint64 winId, const DFMEntryFileInfoPointer info, ActionAfterMount act = kEnterDirectory);
    void mountDevice(quint64 winId, const QString &id, const QString &shellId, ActionAfterMount act = kEnterDirectory);

    void actEject(const QUrl &url);
    void actOpenInNewWindow(quint64 winId, DFMEntryFileInfoPointer info);
    void actOpenInNewTab(quint64 winId, DFMEntryFileInfoPointer info);
    void actMount(quint64 winId, DFMEntryFileInfoPointer info, bool enterAfterMounted = false);
    void actUnmount(DFMEntryFileInfoPointer info);
    void actSafelyRemove(DFMEntryFileInfoPointer info);
    void actRename(quint64 winId, DFMEntryFileInfoPointer info, bool triggerFromSidebar);
    void actFormat(quint64 winId, DFMEntryFileInfoPointer info);
    void actRemove(DFMEntryFileInfoPointer info);
    void actProperties(quint64 winId, DFMEntryFileInfoPointer info);
    void actLogoutAndForgetPasswd(DFMEntryFileInfoPointer info);
    void actErase(DFMEntryFileInfoPointer info);

Q_SIGNALS:
    void requestRename(quint64 winId, const QUrl &url);
    void updateItemAlias(const QUrl &url);

private:
    explicit ComputerController(QObject *parent = nullptr);
    void waitUDisks2DataReady(const QString &id);
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERCONTROLLER_H
