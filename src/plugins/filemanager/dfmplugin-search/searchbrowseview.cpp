/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchbrowseview.h"

#include "dfm-base/widgets/dfmfileview/filesortfilterproxymodel.h"
#include "dfm-base/widgets/dfmfileview/iconitemdelegate.h"
#include "dfm-base/widgets/dfmfileview/listitemdelegate.h"
#include "services/common/menu/menuservice.h"

#include <dfm-framework/framework.h>

SearchBrowseView::SearchBrowseView(QWidget *parent)
    : BrowseView(parent)
{
    auto proxyModel = new FileSortFilterProxyModel(this);
    proxyModel->setSourceModel(new SearchFileViewModel(this));
    setModel(proxyModel);
    setDelegate(QListView::IconMode, new IconItemDelegate(this));
    setDelegate(QListView::ListMode, new ListItemDelegate(this));
    setViewMode(QListView::ListMode);
}

void SearchBrowseView::setRootUrl(const QUrl &url)
{
    model()->setRootUrl(url);
}

QUrl SearchBrowseView::rootUrl()
{
    return model()->rootUrl();
}

void SearchBrowseView::contextMenuEvent(QContextMenuEvent *event)
{
}
