/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "recentbrowseview.h"

#include "dfm-base/widgets/dfmfileview/iconitemdelegate.h"
#include "dfm-base/widgets/dfmfileview/listitemdelegate.h"
#include "services/common/menu/menuservice.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE

RecentBrowseView::RecentBrowseView(QWidget *parent)
    :BrowseView(parent)
{
    setModel(new RecentBrowseViewModel(this));
    setDelegate(QListView::IconMode,new IconItemDelegate(this));
    setDelegate(QListView::ListMode,new ListItemDelegate(this));
    setViewMode(QListView::ListMode);
}

void RecentBrowseView::setRootUrl(const QUrl &url)
{

}

QUrl RecentBrowseView::rootUrl()
{
    return RecentUtil::onlyRootUrl();
}

void RecentBrowseView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);

    auto &ctx = dpfInstance.serviceContext();
    MenuService* menuService = ctx.service<MenuService>(MenuService::name());
    qCCritical(RecentPlugin) << Q_FUNC_INFO << menuService;
    if (!menuService) {
        abort();
    }

    QMenu *tempMenu = menuService->createMenu(AbstractFileMenu::Empty,
                                              RecentUtil::onlyRootUrl(), {}, {});
    if (tempMenu) {
        tempMenu->exec(QCursor::pos());
        delete tempMenu;
    }
}
