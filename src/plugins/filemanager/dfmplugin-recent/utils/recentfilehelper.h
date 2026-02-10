// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTFILEHELPER_H
#define RECENTFILEHELPER_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>

#include <QUrl>
#include <QMimeData>
#include <QFileDevice>

DPRECENT_BEGIN_NAMESPACE
class RecentFileHelper : public QObject
{
    Q_DISABLE_COPY(RecentFileHelper)

public:
    static RecentFileHelper *instance();

public Q_SLOTS:
    bool setPermissionHandle(const quint64 windowId,
                             const QUrl url,
                             const QFileDevice::Permissions permissions,
                             bool *ok,
                             QString *error);
    bool cutFile(const quint64 windowId, const QList<QUrl> sources,
                 const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool copyFile(const quint64, const QList<QUrl>,
                  const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags);
    bool moveToTrash(const quint64 windowId, const QList<QUrl> sources,
                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool openFileInPlugin(quint64 winId, QList<QUrl> urls);
    bool writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                              const QList<QUrl> urls);
    bool openFileInTerminal(const quint64 windowId, const QList<QUrl> urls);

private:
    explicit RecentFileHelper(QObject *parent = nullptr);
};
DPRECENT_END_NAMESPACE

#endif   // RECENTFILEHELPER_H
