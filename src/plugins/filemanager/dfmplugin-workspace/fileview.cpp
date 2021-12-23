/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"
#include "filesortfilterproxymodel.h"
#include "fileselectionmodel.h"
#include "fileviewmodel.h"
#include "baseitemdelegate.h"
#include "iconitemdelegate.h"
#include "listitemdelegate.h"

#include <QResizeEvent>
#include <QScrollBar>

FileView::FileView(QWidget *parent)
    : DListView(parent), d(new FileViewPrivate(this))
{
    setResizeMode(QListView::Adjust);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    setTextElideMode(Qt::ElideMiddle);
    setAlternatingRowColors(false);

    initializeModel();
    initializeDelegate();

    // TODO(liuyangming): init data from config
    QAbstractItemView::model()->sort(0);
}

QWidget *FileView::widget() const
{
    return const_cast<FileView *>(this);
}

void FileView::setViewMode(QListView::ViewMode mode)
{
    if (viewMode() == mode)
        return;

    QListView::setViewMode(mode);
    setItemDelegate(d->delegates[mode]);

    switch (mode) {
    case QListView::IconMode:
        d->initIconModeView();
        break;
    case QListView::ListMode:
        if (model())
            setMinimumWidth(model()->columnCount() * GlobalPrivate::kListViewMinimumWidth);
        d->initListModeView();
        break;
    }
}

void FileView::setDelegate(QListView::ViewMode mode, BaseItemDelegate *view)
{
    if (!view)
        return;

    auto delegate = d->delegates[mode];
    if (delegate) {
        if (delegate->parent())
            delegate->setParent(nullptr);
        delete delegate;
    }

    d->delegates[mode] = view;
}

bool FileView::setRootUrl(const QUrl &url)
{
    model()->setRootUrl(url);
    return true;
}

QUrl FileView::rootUrl() const
{
    return model()->rootUrl();
}

dfmbase::AbstractBaseView::ViewState FileView::viewState() const
{
    // TODO(zhangs): return model state
    return AbstractBaseView::viewState();
}

QList<QAction *> FileView::toolBarActionList() const
{
    // TODO(zhangs): impl me
    return QList<QAction *>();
}

QList<QUrl> FileView::selectedUrlList() const
{
    // TODO(zhangs): impl me
    return QList<QUrl>();
}

void FileView::refresh()
{
    // TODO(zhangs): model()->refresh();
}

FileViewModel *FileView::model() const
{
    auto model = qobject_cast<FileSortFilterProxyModel *>(QAbstractItemView::model());
    if (model)
        return qobject_cast<FileViewModel *>(model->sourceModel());

    return nullptr;
}

void FileView::setModel(QAbstractItemModel *model)
{
    if (model->parent() != this)
        model->setParent(this);
    auto curr = FileView::model();
    if (curr)
        delete curr;
    DListView::setModel(model);
    QObject::connect(this, &FileView::clicked, this, &FileView::onClicked, Qt::UniqueConnection);
}

int FileView::getColumnWidth(const int &column) const
{
    if (d->headerView)
        return d->headerView->sectionSize(column);

    return GlobalPrivate::kListViewDefaultWidth;
}

int FileView::getHeaderViewWidth() const
{
    if (d->headerView)
        return d->headerView->sectionsTotalWidth();

    return 0;
}

void FileView::onHeaderViewMouseReleased()
{
    if (d->headerView->sectionsTotalWidth() != width())
        d->allowedAdjustColumnSize = false;

    // TODO(liuyangming): save data to config
}

void FileView::onHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)

    // TODO(liuyangming): save data to config

    update();
}

void FileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    auto proxyModel = qobject_cast<FileSortFilterProxyModel *>(QAbstractItemView::model());
    proxyModel->setSortRole(model()->getRoleByColumn(logicalIndex));

    proxyModel->sort(logicalIndex, order);

    //TODO liuyangming: save data to config
}

void FileView::onClicked(const QModelIndex &index)
{
    auto item = FileView::model()->itemFromIndex(index);
    if (item)
        Q_EMIT urlClicked(item->url());

    if (item->fileinfo()->isDir())
        Q_EMIT dirClicked(item->url());

    if (item->fileinfo()->isFile())
        Q_EMIT fileClicked(item->url());
}

void FileView::keyPressEvent(QKeyEvent *event)
{
    // TODO(zhangs): impl me

    DListView::keyPressEvent(event);
}

void FileView::resizeEvent(QResizeEvent *event)
{
    if (d->headerView) {
        if (qAbs(d->headerView->sectionsTotalWidth() - width()) < 10)
            d->allowedAdjustColumnSize = true;

        d->updateListModeColumnWidth();
    }

    return DListView::resizeEvent(event);
}

QModelIndexList FileView::selectedIndexes() const
{
    return qobject_cast<FileSelectionModel *>(selectionModel())->selectedIndexes();
}

void FileView::initializeModel()
{
    auto model = new FileViewModel(this);
    auto proxyModel = new FileSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    setModel(proxyModel);

    auto selectionModel = new FileSelectionModel(model);
    setSelectionModel(selectionModel);
}

void FileView::initializeDelegate()
{
    setDelegate(QListView::ViewMode::IconMode, new IconItemDelegate(this));
    setDelegate(QListView::ViewMode::ListMode, new ListItemDelegate(this));
}
