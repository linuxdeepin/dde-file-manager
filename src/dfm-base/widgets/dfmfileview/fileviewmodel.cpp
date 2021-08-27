/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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

DFMFileViewModelPrivate::DFMFileViewModelPrivate(DFMFileViewModel *qq)
    :q_ptr(qq)
{

}

DFMFileViewModelPrivate::~DFMFileViewModelPrivate()
{

}

DFMFileViewModel::DFMFileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel (parent)
    , d_ptr(new DFMFileViewModelPrivate(this))
{

}

QModelIndex DFMFileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const DFMFileViewModel);

    if(row < 0 || column < 0 || d->m_childers.size() <= row)
        return  QModelIndex();

    return createIndex(row,column,const_cast<DFMFileViewItem*>(d->m_childers.at(row).data()));
}

QModelIndex DFMFileViewModel::setRootUrl(const QUrl &url)
{
    Q_D(DFMFileViewModel);
    if (!d->m_root.isNull() && d->m_root->url() == url)
        return createIndex(-1, 0, &d->m_root);

    d->m_root.reset(new DFMFileViewItem(url));
    QModelIndex root = createIndex(-1, 0, &d->m_root);

    if(!url.isValid())
        return root;

    d->m_childers.clear();

    if (d->m_column == 0)
        d->m_column = 4;

    if (!d->m_watcher.isNull()) {
        disconnect(d->m_watcher.data());
    }
    d->m_watcher = DFMWacherFactory::instance().create<DAbstractFileWatcher>(url);
    if (d->m_watcher.isNull()) {
        connect(d->m_watcher.data(), &DAbstractFileWatcher::fileMoved, this, &DFMFileViewModel::dofileMoved);
        connect(d->m_watcher.data(), &DAbstractFileWatcher::fileDeleted, this, &DFMFileViewModel::doFileDeleted);
        connect(d->m_watcher.data(), &DAbstractFileWatcher::subfileCreated, this, &DFMFileViewModel::dofileCreated);
        connect(d->m_watcher.data(), &DAbstractFileWatcher::fileModified, this, &DFMFileViewModel::dofileModified);
        connect(d->m_watcher.data(), &DAbstractFileWatcher::fileAttributeChanged, this, &DFMFileViewModel::dofileModified);
    }

    return root;
}

QUrl DFMFileViewModel::rootUrl()
{
    Q_D(DFMFileViewModel);
    return d->m_root->fileinfo<DAbstractFileInfo>()->url();
}

DAbstractFileInfoPointer DFMFileViewModel::fileInfo(const QModelIndex &index)
{
    Q_D(DFMFileViewModel);
    if (!index.isValid())
        return nullptr;
    if(index.row() < 0  || d->m_childers.size() <= index.row())
        return  nullptr;
    return d->m_childers.at(index.row())->fileinfo<DAbstractFileInfo>();
}

QModelIndex DFMFileViewModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
    //    return createIndex(-1, 0, const_cast<DFMFileViewItem*>(&m_root));
}

int DFMFileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_D(const DFMFileViewModel);
    return d->m_childers.size();
}

int DFMFileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_D(const DFMFileViewModel);
    return d->m_column;
}

QVariant DFMFileViewModel::data(const QModelIndex &index, int role) const
{
    Q_D(const DFMFileViewModel);
    return d->m_childers.at(index.row())->data(role);
}

void DFMFileViewModel::doUpdateChildren(const QList<QSharedPointer<DFMFileViewItem> > &children)
{
    Q_D(DFMFileViewModel);

    beginResetModel();
    d->m_childers.setList(children);
    endResetModel();
}


void DFMFileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    Q_D(DFMFileViewModel);

    if (!d->m_traversalThread.isNull()) {
        d->m_traversalThread->quit();
        d->m_traversalThread->wait();
        disconnect(d->m_traversalThread.data());
    }
    d->m_traversalThread.reset(new DFMTraversalDirThread(
                                   d->m_root->url(),QStringList(),
                                   QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System,
                                   QDirIterator::NoIteratorFlags));
    if (d->m_traversalThread.isNull())
        return;
    connect(d->m_traversalThread.data(),&DFMTraversalDirThread::updateChildren, this, &DFMFileViewModel::doUpdateChildren,
            Qt::QueuedConnection);
    d->m_traversalThread->start();
}


bool DFMFileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return true;
}
