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

#include "fileviewmodel.h"
#include "private/fileviewmodel_p.h"

DFMBASE_BEGIN_NAMESPACE
FileViewModelPrivate::FileViewModelPrivate(FileViewModel *qq)
    : QObject (qq)
    , q_ptr(qq)
{

}

FileViewModelPrivate::~FileViewModelPrivate()
{

}

void FileViewModelPrivate::doUpdateChildren(const QList<QSharedPointer<FileViewItem> > &children)
{
    q_ptr->beginResetModel();
    childers.setList(children);
    q_ptr->endResetModel();
}

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel (parent)
    , d(new FileViewModelPrivate(this))
{

}

FileViewModel::~FileViewModel()
{

}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const FileViewModel);

    if(row < 0 || column < 0 || d->childers.size() <= row)
        return  QModelIndex();

    return createIndex(row,column,const_cast<FileViewItem*>(d->childers.at(row).data()));
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    Q_D(FileViewModel);
    if (!d->root.isNull() && d->root->url() == url)
        return createIndex(-1, 0, &d->root);

    d->root.reset(new FileViewItem(url));
    QModelIndex root = createIndex(-1, 0, &d->root);

    if(!url.isValid())
        return root;

    d->childers.clear();

    if (d->column == 0)
        d->column = 4;

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data());
    }
    d->watcher = WacherFactory::create<AbstractFileWatcher>(url);
    if (d->watcher.isNull()) {
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                         d, &FileViewModelPrivate::doFileDeleted);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                         d, &FileViewModelPrivate::dofileCreated);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                         d, &FileViewModelPrivate::dofileModified);
    }

    return root;
}

QUrl FileViewModel::rootUrl()
{
    Q_D(FileViewModel);
    return d->root->fileinfo()->url();
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index)
{
    Q_D(FileViewModel);
    if (!index.isValid())
        return nullptr;
    if(index.row() < 0  || d->childers.size() <= index.row())
        return  nullptr;
    return d->childers.at(index.row())->fileinfo();
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
    //    return createIndex(-1, 0, const_cast<DFMFileViewItem*>(&m_root));
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_D(const FileViewModel);
    return d->childers.size();
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_D(const FileViewModel);
    return d->column;
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    Q_D(const FileViewModel);
    return d->childers.at(index.row())->data(role);
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    Q_D(FileViewModel);

    if (!d->traversalThread.isNull()) {
        d->traversalThread->quit();
        d->traversalThread->wait();
        disconnect(d->traversalThread.data());
    }
    d->traversalThread.reset(new TraversalDirThread(
                                   d->root->url(),QStringList(),
                                   QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System,
                                   QDirIterator::NoIteratorFlags));
    if (d->traversalThread.isNull())
        return;
    QObject::connect(d->traversalThread.data(),& TraversalDirThread::updateChildren,
            d, &FileViewModelPrivate::doUpdateChildren,
            Qt::QueuedConnection);
    d->traversalThread->start();
}


bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return true;
}
DFMBASE_END_NAMESPACE
