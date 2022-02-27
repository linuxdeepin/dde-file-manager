/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef WORKSPACEMENU_H
#define WORKSPACEMENU_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"

DFMBASE_USE_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class WorkspaceMenu : public AbstractMenu
{
public:
    explicit WorkspaceMenu(QObject *parent = nullptr);
    QMenu *build(QWidget *parent,
                 MenuMode mode,
                 const QUrl &rootUrl,
                 const QUrl &foucsUrl,
                 const QList<QUrl> &selected = {},
                 QVariant customData = QVariant()) override;

private:
    void actionBusiness(QAction *act) override;

    void assemblesEmptyAreaActions(QMenu *menu, const QUrl &rootUrl);
    void assemblesNormalActions(QMenu *menu, const QUrl &rootUrl, const QUrl &foucsUrl, const QList<QUrl> &selectList);
    void addActionsToMenu(QMenu *menu, const QVector<ActionDataContainer> &dataList);
    void transTypesToActionsData(const QVector<ActionType> &typeList, QVector<ActionDataContainer> &dataList);
    void assemblesSubActions(QVector<ActionDataContainer> &dataList);

    int getRoleByActionType(const ActionType type) const;

    FileView *view { nullptr };
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEMENU_H
