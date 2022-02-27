/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef RECENTMANAGER_H
#define RECENTMANAGER_H

#include "dfmplugin_recent_global.h"
#include "recentfileinfo.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QQueue>
#include <QThread>
#include <QTimer>

DPRECENT_BEGIN_NAMESPACE

class RecentManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RecentManager)

public:
    static RecentManager *instance();

    inline static QString scheme()
    {
        return "recent";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("document-open-recent-symbolic");
    }

    inline static QString xbelPath()
    {
        return QDir::homePath() + "/.local/share/recently-used.xbel";
    }

    static QUrl rootUrl();

    static void clearRecent();
    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static bool openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error);

    static bool writeToClipBoardHandle(const quint64 windowId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);
    QMap<QUrl, AbstractFileInfoPointer> getRecentNodes() const;
    bool removeRecentFile(const QUrl &url);

signals:
    void asyncHandleFileChanged();

private:
    explicit RecentManager(QObject *parent = nullptr);
    ~RecentManager() override;
    void init();

private slots:
    void updateRecent();
    void onUpdateRecentFileInfo(const QUrl &url, qint64 readTime);
    void onDeleteExistRecentUrls(QList<QUrl> &urls);

public:
    // services instance
    static DSB_FM_NAMESPACE::WindowsService *winServIns();
    static DSB_FM_NAMESPACE::TitleBarService *titleServIns();
    static DSB_FM_NAMESPACE::SideBarService *sideBarServIns();
    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServIns();
    static DSC_NAMESPACE::FileOperationsService *fileOperationsServIns();

private:
    QTimer updateRecentTimer;
    QThread workerThread;
    AbstractFileWatcherPointer watcher;
    QMap<QUrl, AbstractFileInfoPointer> recentNodes;
};

DPRECENT_END_NAMESPACE
#endif   // RECENTMANAGER_H
