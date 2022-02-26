/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "views/headerview.h"
#include "views/fileviewstatusbar.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/dragdrophelper.h"
#include "utils/viewdrawhelper.h"
#include "utils/selecthelper.h"
#include "utils/shortcuthelper.h"
#include "utils/fileoperaterhelper.h"
#include "utils/fileviewmenuhelper.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewPrivate::FileViewPrivate(FileView *qq)
    : q(qq)
{
    dragDropHelper = new DragDropHelper(qq);
    viewDrawHelper = new ViewDrawHelper(qq);
    selectHelper = new SelectHelper(qq);
    shortcutHelper = new ShortcutHelper(qq);
    viewMenuHelper = new FileViewMenuHelper(qq);
}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    int horizontalMargin = 0;

    int contentWidth = q->maximumViewportSize().width();

    if (itemWidth <= 0)
        itemWidth = q->itemSizeHint().width() + q->spacing() * 2;

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

    if (statusBar) {
        statusBar->setScalingVisible(true);
        q->setIconSize(QSize(kIconSizeList[currentIconSizeLevel],
                             kIconSizeList[currentIconSizeLevel]));
        statusBar->scalingSlider()->setValue(currentIconSizeLevel);
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

    q->setIconSize(QSize(GlobalPrivate::kListViewIconSize, GlobalPrivate::kListViewIconSize));
    updateListModeColumnWidth();

    if (statusBar)
        statusBar->setScalingVisible(false);
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

QModelIndexList FileViewPrivate::selectedDraggableIndexes()
{
    QModelIndexList indexes = q->selectedIndexes();

    auto isNotDragEnabled = [=](const QModelIndex &index) {
        return !(q->model()->flags(index) & Qt::ItemIsDragEnabled);
    };

    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), isNotDragEnabled),
                  indexes.end());

    return indexes;
}

void FileViewPrivate::initContentLabel()
{
    if (!contentLabel) {
        contentLabel = new QLabel(q);

        QPalette palette = contentLabel->palette();
        QStyleOption opt;
        opt.initFrom(contentLabel);
        QColor color = opt.palette.color(QPalette::Inactive, QPalette::Text);
        palette.setColor(QPalette::Text, color);
        contentLabel->setPalette(palette);

        auto font = contentLabel->font();
        font.setFamily("SourceHanSansSC-Light");
        font.setPixelSize(20);
        contentLabel->setFont(font);

        contentLabel.setCenterIn(q);
        contentLabel->setStyleSheet(q->styleSheet());
        contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        contentLabel->show();
    }
}
