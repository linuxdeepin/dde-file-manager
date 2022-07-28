/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef RECENTFILEHELPER_H
#define RECENTFILEHELPER_H

#include "dfmplugin_recent_global.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"

#include <QUrl>
#include <QMimeData>
#include <QFileDevice>

DPRECENT_BEGIN_NAMESPACE
class RecentFileHelper : public QObject
{
    Q_DISABLE_COPY(RecentFileHelper)
public:
    static RecentFileHelper *instance();
    inline static QString scheme()
    {
        return "recent";
    }

    static void removeRecent(const QList<QUrl> &urls);

    static bool openFileLocation(const QUrl &url);

    static void openFileLocation(const QList<QUrl> &urls);

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
    bool linkFile(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence);
    bool writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                              const QList<QUrl> urls);
    bool openFileInTerminal(const quint64 windowId, const QList<QUrl> urls);

private:
    explicit RecentFileHelper(QObject *parent = nullptr);
};
DPRECENT_END_NAMESPACE

#endif   // RECENTFILEHELPER_H
