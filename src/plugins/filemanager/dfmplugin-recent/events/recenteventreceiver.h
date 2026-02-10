// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTEVENTRECEIVER_H
#define RECENTEVENTRECEIVER_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QUrl>

namespace dfmplugin_recent {

class RecentEventReceiver : public QObject
{
    Q_OBJECT

public:
    static RecentEventReceiver *instance();

public slots:
    void handleWindowUrlChanged(quint64 winId, const QUrl &url);
    void handleRemoveFilesResult(const QList<QUrl> &urls, bool ok, const QString &errMsg);
    void handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg);
    void handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg);

    bool customColumnRole(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool customRoleDisplayName(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool isTransparent(const QUrl &url, DFMGLOBAL_NAMESPACE::TransparentStatus *status);
    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);
    bool handlePropertydialogDisable(const QUrl &url);

private:
    explicit RecentEventReceiver(QObject *parent = nullptr);
};

}

#endif   // RECENTEVENTRECEIVER_H
