/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

#include <QResizeEvent>
#include <QScrollBar>

DFMBASE_BEGIN_NAMESPACE
FileView::FileView(QWidget *parent)
    : DListView(parent)
    , d(new FileViewPrivate(this))
{
    auto model = new FileViewModel(this);
    this->setCornerWidget(d->headview);
    setModel(model);
    setDelegate(QListView::ViewMode::IconMode, new IconItemDelegate(this));
    setDelegate(QListView::ViewMode::ListMode, new ListItemDelegate(this));
    setViewMode(QListView::ViewMode::IconMode);
}

void FileView::setViewMode(QListView::ViewMode mode)
{
    QListView::setViewMode(mode);
    setItemDelegate(d->delegates[mode]);
}

void FileView::setDelegate(QListView::ViewMode mode, QAbstractItemDelegate *view)
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

void FileView::setRootUrl(const QUrl &url)
{
    model()->setRootUrl(url);
}

QUrl FileView::rootUrl()
{
    return model()->rootUrl();
}

FileViewModel *FileView::model()
{
    return qobject_cast<FileViewModel *>(QAbstractItemView::model());
}

void FileView::setModel(QAbstractItemModel *model)
{
    if (model->parent() != this)
        model->setParent(this);
    auto curr = FileView::model();
    if (curr)
        delete curr;
    DListView::setModel(model);
    QObject::connect(this, &FileView::clicked,
                     this ,[=](const QModelIndex &index){
        auto item = FileView::model()->itemFromIndex(index);
        if (item)
            Q_EMIT urlClicked(item->url());

        if (item->fileinfo()->isDir())
            Q_EMIT dirClicked(item->url());

        if (item->fileinfo()->isFile())
            Q_EMIT fileClicked(item->url());
    },Qt::UniqueConnection);
}

void FileView::resizeEvent(QResizeEvent *event)
{
    return DListView::resizeEvent(event);
}

DFMBASE_END_NAMESPACE

