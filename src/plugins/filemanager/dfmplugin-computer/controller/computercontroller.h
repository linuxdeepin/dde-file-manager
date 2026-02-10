// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERCONTROLLER_H
#define COMPUTERCONTROLLER_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>
#include <QObject>

#define ComputerControllerInstance DPCOMPUTER_NAMESPACE::ComputerController::instance()

namespace dfmplugin_computer {

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
    bool doSetProtocolDeviceAlias(DFMEntryFileInfoPointer info, const QString &alias);

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
    void actProperties(quint64 winId, DFMEntryFileInfoPointer info);
    void actLogoutAndForgetPasswd(DFMEntryFileInfoPointer info);
    void actErase(DFMEntryFileInfoPointer info);

Q_SIGNALS:
    void requestRename(quint64 winId, const QUrl &url);
    void updateItemAlias(const QUrl &url, const QString &alias, bool isProtocol);

private:
    explicit ComputerController(QObject *parent = nullptr);
    void waitUDisks2DataReady(const QString &id);
    void handleUnAccessableDevCdCall(quint64 winId, DFMEntryFileInfoPointer info);
    void handleNetworkCdCall(quint64 winId, DFMEntryFileInfoPointer info);
};

}

#endif   // COMPUTERCONTROLLER_H
