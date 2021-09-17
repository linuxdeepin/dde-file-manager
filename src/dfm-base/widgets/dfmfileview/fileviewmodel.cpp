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

#include <QApplication>

DFMBASE_BEGIN_NAMESPACE

FileViewModelPrivate::FileViewModelPrivate(FileViewModel *qq)
    : QObject (qq)
    , q(qq)
{

}

FileViewModelPrivate::~FileViewModelPrivate()
{

}

void FileViewModelPrivate::doUpdateChildren(const QList<FileViewItem*> &children)
{
    q->beginResetModel();
    childers.setList(children);
    q->endResetModel();
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel (parent)
    , d(new FileViewModelPrivate(this))
{

}

FileViewModel::~FileViewModel()
{
    clear();
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(row < 0 || column < 0 || d->childers.size() <= row)
        return  QModelIndex();

    return createIndex(row, column, d->childers.at(row));
}

const FileViewItem *FileViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (0 > index.row() || index.row() >= d->childers.size())
        return nullptr;
    return d->childers.at(index.row());
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    if (!d->root.isNull() && d->root->url() == url) {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        return createIndex(-1, 0, &d->root);
    }

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
                         d.data(), &FileViewModelPrivate::doFileDeleted);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                         d.data(), &FileViewModelPrivate::dofileCreated);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                         d.data(), &FileViewModelPrivate::dofileModified);
    }

    d->canFetchMoreFlag = true;
    fetchMore(root);
    return root;
}

QUrl FileViewModel::rootUrl()
{
    return d->root->fileinfo()->url();
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index)
{
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
    return d->childers.size();
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->column;
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    auto item = itemFromIndex(index);
    if (item)
        return item->data(role);
    return QVariant();
}

void FileViewModel::clear()
{
    for (int x = 0; x < columnCount(); x++) {
        for (int y = 0; x < rowCount(); y++){
            delete itemFromIndex(index(x,y));
        }
    }
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
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
    QObject::connect(d->traversalThread.data(), &TraversalDirThread::updateChildren,
                     d.data(), &FileViewModelPrivate::doUpdateChildren,
                     Qt::QueuedConnection);

    if (d->canFetchMoreFlag) {
        d->canFetchMoreFlag = false;
        d->traversalThread->start();
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->canFetchMoreFlag;
}
DFMBASE_END_NAMESPACE
