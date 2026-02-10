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
