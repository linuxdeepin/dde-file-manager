/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DETAILSPACEHELPER_H
#define DETAILSPACEHELPER_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QMap>
#include <QMutex>
#include <QObject>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceWidget;
class WorkspaceHelper : public QObject
{
    Q_OBJECT
public:
    enum class DirOpenMode {
        kOpenInCurrentWindow,
        kOpenNewWindow,
        //kForceOpenNewWindow // Todo(yanghao): ???
    };

    static WorkspaceHelper *instance();
    WorkspaceWidget *findWorkspaceByWindowId(quint64 windowId);
    void addWorkspace(quint64 windowId, WorkspaceWidget *workspace);
    void removeWorkspace(quint64 windowId);
    quint64 windowId(QWidget *sender);
    void switchViewMode(quint64 windowId, int viewMode);
    void addScheme(const QString &scheme);
    void openUrlInNewTab(quint64 windowId, const QUrl &viewMode);

    void actionShowFilePreviewDialog(const QList<QUrl> &urls);
    void actionNewWindow(const QList<QUrl> &urls);
    void actionNewTab(quint64 windowId, const QUrl &url);
    void actionHiddenFiles(quint64 windowId, const QUrl &url);
    void actionOpen(quint64 windowId, const QList<QUrl> &urls, const DirOpenMode openMode = DirOpenMode::kOpenInCurrentWindow);
    void actionProperty(quint64 windowId, const QList<QUrl> &urls);
    void actionDeleteFiles(quint64 windowId, const QList<QUrl> &urls);
    void actionOpenInTerminal(quint64 windowId, const QList<QUrl> &urls);
    void actionNewFolder(quint64 windowId, const QUrl &url);
    void actionRenameFile(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl);
    void actionWriteToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls);
    void actionPastFiles(const quint64 windowId, const ClipBoard::ClipboardAction action,
                         const QList<QUrl> &sourceUrls, const QUrl &target,
                         const AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint);

signals:
    void viewModeChanged(quint64 windowId, int viewMode);
    void openNewTab(quint64 windowId, const QUrl &url);

private:
    explicit WorkspaceHelper(QObject *parent = nullptr);
    static QMutex &mutex();
    static QMap<quint64, WorkspaceWidget *> kWorkspaceMap;
    Q_DISABLE_COPY(WorkspaceHelper)
};

DPWORKSPACE_END_NAMESPACE

#endif   // DETAILSPACEHELPER_H
