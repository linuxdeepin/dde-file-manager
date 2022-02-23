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
#include "workspace/workspace_defines.h"

#include <QMap>
#include <QMutex>
#include <QObject>

#include <functional>

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE
class CustomTopWidgetInterface;
class WorkspaceWidget;
class WorkspaceHelper : public QObject
{
    Q_OBJECT
public:
    static WorkspaceHelper *instance();

    using KeyType = QString;
    using TopWidgetCreator = std::function<CustomTopWidgetInterface *()>;
    using TopWidgetCreatorMap = QMap<KeyType, TopWidgetCreator>;

    void registerTopWidgetCreator(const KeyType &scheme, const TopWidgetCreator &creator);
    bool isRegistedTopWidget(const KeyType &scheme) const;
    CustomTopWidgetInterface *createTopWidgetByUrl(const QUrl &url);
    void setCustomTopWidgetVisible(quint64 windowId, const QString &scheme, bool visible);
    void setFilterData(quint64 windowId, const QUrl &url, const QVariant &data);
    void setFilterCallback(quint64 windowId, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);

    WorkspaceWidget *findWorkspaceByWindowId(quint64 windowId);
    void closeTab(const QString &path);
    void addWorkspace(quint64 windowId, WorkspaceWidget *workspace);
    void removeWorkspace(quint64 windowId);
    quint64 windowId(const QWidget *sender);
    void switchViewMode(quint64 windowId, int viewMode);
    void addScheme(const QString &scheme);
    void openUrlInNewTab(quint64 windowId, const QUrl &viewMode);
    void actionNewWindow(const QList<QUrl> &urls);
    void actionNewTab(quint64 windowId, const QUrl &url);

signals:
    void viewModeChanged(quint64 windowId, int viewMode);
    void openNewTab(quint64 windowId, const QUrl &url);
    void requestSetViewFilterData(quint64 windowId, const QUrl &url, const QVariant &data);
    void requestSetViewFilterCallback(quint64 windowId, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);

private:
    explicit WorkspaceHelper(QObject *parent = nullptr);
    static QMutex &mutex();
    static QMap<quint64, WorkspaceWidget *> kWorkspaceMap;

private:
    TopWidgetCreatorMap topWidgetCreators;
    Q_DISABLE_COPY(WorkspaceHelper)
};

DPWORKSPACE_END_NAMESPACE

#endif   // DETAILSPACEHELPER_H
