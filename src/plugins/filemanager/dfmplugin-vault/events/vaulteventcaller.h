// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTEVENTCALLER_H
#define VAULTEVENTCALLER_H

#include "dfmplugin_vault_global.h"

#include <QUrl>

namespace dfmplugin_vault {
class VaultEventCaller
{
    VaultEventCaller() = delete;

public:
    static void sendItemActived(quint64 windowId, const QUrl &url);
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendVaultProperty(const QUrl &url);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
};
}
#endif   // VAULTEVENTCALLER_H
