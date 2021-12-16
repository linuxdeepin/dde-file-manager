/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "fileview_p.h"
#include "headerview.h"
#include "fileviewmodel.h"

FileViewPrivate::FileViewPrivate(FileView *qq)
    : q(qq)
{
}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    int horizontalMargin = 0;
    int contentWidth = q->maximumViewportSize().width();
    return qMax((contentWidth - horizontalMargin - 1) / itemWidth, 1);
}

QUrl FileViewPrivate::modelIndexUrl(const QModelIndex &index) const
{
    return index.data().toUrl();
}

void FileViewPrivate::initIconModeView()
{
    if (headerView) {
        headerView->disconnect();
        q->removeHeaderWidget(0);
        delete headerView;
        headerView = nullptr;
    }
}

void FileViewPrivate::initListModeView()
{
    if (!headerView) {
        headerView = new HeaderView(Qt::Orientation::Horizontal, q);

        headerView->setHighlightSections(false);
        headerView->setSectionsClickable(true);
        headerView->setSortIndicatorShown(true);
        headerView->setSectionResizeMode(QHeaderView::Interactive);
        headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        headerView->setCascadingSectionResizes(false);
        headerView->setFixedHeight(GlobalPrivate::kListViewIconSize);
        headerView->setMinimumSectionSize(GlobalPrivate::kListViewMinimumWidth);

        headerView->setModel(q->model());
        if (q->selectionModel()) {
            headerView->setSelectionModel(q->selectionModel());
        }
    }

    q->addHeaderWidget(headerView);

    QObject::connect(headerView, &HeaderView::mouseReleased, q, &FileView::onHeaderViewMouseReleased);
    QObject::connect(headerView, &HeaderView::sectionResized, q, &FileView::onHeaderSectionResized);
    QObject::connect(headerView, &HeaderView::sortIndicatorChanged, q, &FileView::onSortIndicatorChanged);

    updateListModeColumnWidth();
}

void FileViewPrivate::updateListModeColumnWidth()
{
    if (!allowedAdjustColumnSize)
        return;
    if (!headerView)
        return;

    int nameColumnWidth = q->width();
    for (int i = 1; i < headerView->model()->columnCount(); ++i) {
        int columnWidth = q->model()->getColumnWidth(i);
        headerView->resizeSection(i, columnWidth);
        nameColumnWidth -= columnWidth;
    }
    headerView->resizeSection(0, nameColumnWidth);
}
