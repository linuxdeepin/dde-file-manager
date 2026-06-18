// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTEVENTRECEIVER_H
#define VAULTEVENTRECEIVER_H

#include "dfmplugin_vault_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_vault {
class VaultEventReceiver : public QObject
{
    Q_OBJECT
private:
    VaultEventReceiver(QObject *parent = nullptr);
    //! Re-entry guard: prevents recursive calls to changeUrlEventFilter during vault unlock.
    //! When the vault is encrypted and unlockVaultDialog is called, the synchronous unlock flow
    //! (waitForFinished on cryfs process) spins a nested Qt event loop. If a kChangeCurrentUrl
    //! event is dispatched inside that loop, changeUrlEventFilter is called again before the
    //! first invocation finishes. The filesystem check (state(useCache=false)) still sees the
    //! vault as encrypted because cryfs hasn't completed mounting, causing another unlockVaultDialog
    //! call and infinite recursion until the stack is exhausted.
    static bool s_urlChangeInUnlock;

public:
    static VaultEventReceiver *instance();

    void connectEvent();

public slots:
    void computerOpenItem(quint64 winId, const QUrl &url);
    bool handleNotAllowedAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    void handleCurrentUrlChanged(const quint64 &winId, const QUrl &url);
    bool handleSideBarItemDragMoveData(const QList<QUrl> &urls, const QUrl &url, Qt::DropAction *action);
    bool handleShortCutPasteFiles(const quint64 &winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool changeUrlEventFilter(quint64 windowId, const QUrl &url);
    bool handlePathtoVirtual(const QList<QUrl> files, QList<QUrl> *virtualFiles);
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action);
    bool handlePermissionViewAsh(const QUrl &url, bool *isAsh);
    bool handleFileCanTaged(const QUrl &url, bool *canTag);
};
}
#endif   // VAULTEVENTRECEIVER_H
