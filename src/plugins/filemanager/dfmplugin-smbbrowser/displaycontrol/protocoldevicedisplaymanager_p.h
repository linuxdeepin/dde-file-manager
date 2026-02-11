// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCOLDEVICEDISPLAYMANAGER_P_H
#define PROTOCOLDEVICEDISPLAYMANAGER_P_H

#include "dfmplugin_smbbrowser_global.h"
#include "datahelper/virtualentrydata.h"

DPSMBBROWSER_BEGIN_NAMESPACE

class ProtocolDeviceDisplayManager;
class ProtocolDeviceDisplayManagerPrivate
{
    friend class ProtocolDeviceDisplayManager;
    ProtocolDeviceDisplayManager *q { nullptr };
    SmbDisplayMode displayMode { SmbDisplayMode::kSeperate };
    bool showOffline { false };

public:
    ProtocolDeviceDisplayManagerPrivate(ProtocolDeviceDisplayManager *qq)
        : q(qq) { }
    ~ProtocolDeviceDisplayManagerPrivate() { }

private:
    void init();

    // on status changed
    void onDisplayModeChanged();
    void onShowOfflineChanged();

    // utils
    bool isSupportVEntry(const QUrl &entryUrl);
    bool isSupportVEntry(const QString &devId);

    void removeAllSmb(QList<QUrl> *entryUrls);
};

DPSMBBROWSER_END_NAMESPACE

#endif   // PROTOCOLDEVICEDISPLAYMANAGER_P_H
