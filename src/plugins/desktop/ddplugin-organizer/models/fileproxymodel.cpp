/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "fileproxymodel_p.h"
#include "interface/fileinfomodelshell.h"
#include "modeldatahandler.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"

#include <QDebug>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

FileProxyModelPrivate::FileProxyModelPrivate(FileProxyModel *qq)
    : QObject(qq)
    , q(qq)
{

}

void FileProxyModelPrivate::reset()
{
    fileList.clear();
    fileMap.clear();

    auto model = q->sourceModel();
    if (!model) {
        qWarning() << "invaild source model.";
        return;
    }

    // for rename
    connect(shell, &FileInfoModelShell::dataReplaced,
            this, &FileProxyModelPrivate::sourceDataRenamed);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &FileProxyModelPrivate::sourceDataChanged);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &FileProxyModelPrivate::sourceRowsInserted);

    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &FileProxyModelPrivate::sourceRowsAboutToBeRemoved);

    connect(model, &QAbstractItemModel::modelAboutToBeReset,
            this, &FileProxyModelPrivate::sourceAboutToBeReset);

    connect(model, &QAbstractItemModel::modelReset,
            this, &FileProxyModelPrivate::sourceReset);
}

void FileProxyModelPrivate::clearMapping()
{
    fileList.clear();
    fileMap.clear();
}

void FileProxyModelPrivate::createMapping()
{
    auto source = q->sourceModel();
    if (!source || !shell)
        return;

    if (!handler) {
        qWarning() << "no handler: clear all datas";
        clearMapping();
        return;
    }

    fileList = handler->acceptReset(shell->files());
    QMap<QUrl, DFMLocalFileInfoPointer> maps;
    for (const QUrl &url : fileList)
        maps.insert(url, shell->fileInfo(shell->index(url)));

    fileMap = maps;
}

void FileProxyModelPrivate::sourceDataChanged(const QModelIndex &sourceTopleft, const QModelIndex &sourceBottomright, const QVector<int> &roles)
{
    if (!sourceTopleft.isValid() || !sourceBottomright.isValid())
        return;

    int begin = qMin(sourceTopleft.row(), sourceBottomright.row());
    int end = qMax(sourceTopleft.row(), sourceBottomright.row());

    QList<QModelIndex> idxs;
    // find items in this model
    for (int i = begin; i <= end; ++i) {
        auto url = shell->fileUrl(q->sourceModel()->index(i, 0));
        auto cur = q->index(url);
        if (cur.isValid())
            idxs << cur;
    }

    if (idxs.isEmpty())
        return;

    // AscendingOrder
    qSort(idxs.begin(), idxs.end(), [](const QModelIndex &t1, const QModelIndex &t2){
        return t1.row() < t2.row();
    });

    emit q->dataChanged(idxs.first(), idxs.last(), roles);
}

void FileProxyModelPrivate::sourceAboutToBeReset()
{
    q->beginResetModel();
}

void FileProxyModelPrivate::sourceReset()
{
    createMapping();
    q->endResetModel();
}

void FileProxyModelPrivate::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    Q_UNUSED(sourceParent)
    if (!handler) {
        qWarning() << "no handler to insert";
        return;
    }

    if ((start < 0) || (end < 0))
        return;

    QList<QUrl> files;
    for (int i = start; i <= end; ++i) {
        auto url = shell->fileUrl(q->sourceModel()->index(i, 0));
        if (!fileMap.contains(url) && handler->acceptInsert(url))
            files << url;
    }

    if (files.isEmpty())
        return;

    int row = fileList.count();
    q->beginInsertRows(q->rootIndex(), row, row + files.count() - 1);

    fileList.append(files);
    for (const QUrl &url : files)
        fileMap.insert(url, shell->fileInfo(shell->index(url)));

    q->endInsertRows();
}

void FileProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    Q_UNUSED(sourceParent)
    if ((start < 0) || (end < 0))
        return;

    QList<QUrl> files;
    for (int i = start; i <= end; ++i) {
        auto url = shell->fileUrl(q->sourceModel()->index(i, 0));
        if (fileMap.contains(url))
            files << url;
    }

    if (files.isEmpty())
        return;

    // remove one by one
    for (const QUrl &url : files) {
        int row = fileList.indexOf(url);
        if (row < 0)
            continue;

        q->beginRemoveRows(q->rootIndex(), row, row);
        fileList.removeAt(row);
        fileMap.remove(url);
        q->endRemoveRows();
    }
}

void FileProxyModelPrivate::sourceDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    int row = fileList.indexOf(oldUrl);
    auto newInfo = shell->fileInfo(shell->index(newUrl));
    bool accept = false;
    if (handler)
        accept = handler->acceptRename(oldUrl, newUrl);
    else
        qWarning() << "no handler to insert reamed file.";

    if (row < 0) { // no old data, need to insert
        if (!fileMap.contains(newUrl) && accept) { // insert it if accept.
            row = fileList.count();
            q->beginInsertRows(q->rootIndex(), row, row);
            fileList.append(newUrl);
            fileMap.insert(newUrl, newInfo);
            q->endInsertRows();
            return;
        }
    } else {
        if (accept) {
            // e.g. a mv to b(b is existed)
            if (fileMap.contains(newUrl)) {
                //! treat as removing if newurl is existed in organizer.
                q->beginRemoveRows(q->rootIndex(), row, row);
                fileList.removeAt(row);
                fileMap.remove(oldUrl);
                q->endRemoveRows();

                row = fileList.indexOf(newUrl);
            } else {
                fileList.replace(row, newUrl);
                fileMap.remove(oldUrl);
                fileMap.insert(newUrl, newInfo);
                emit q->dataReplaced(oldUrl, newUrl);
            }

            auto index = q->sourceModel()->index(row, 0);
            emit q->dataChanged(index, index);
        } else {
            q->beginRemoveRows(q->rootIndex(), row, row);
            fileList.removeAt(row);
            fileMap.remove(oldUrl);
            q->endRemoveRows();
        }
    }
}

void FileProxyModelPrivate::doRefresh(bool global)
{
    if (global) {
        shell->refresh(shell->rootIndex());
    } else {
        sourceAboutToBeReset();
        sourceReset();
        q->update();
    }
}

FileProxyModel::FileProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new FileProxyModelPrivate(this))
{

}

FileProxyModel::~FileProxyModel()
{
    delete d;
    d = nullptr;
}

void FileProxyModel::setModelShell(FileInfoModelShell *shell)
{
    if (auto model = sourceModel()) {
       model->disconnect(this);
       model->disconnect(d);
    }

    beginResetModel();

    d->shell = shell;
    if (shell) {
        QAbstractProxyModel::setSourceModel(shell->sourceModel());
    } else {
        QAbstractProxyModel::setSourceModel(nullptr);
    }

    d->reset();
    endResetModel();
}

void FileProxyModel::setHandler(ModelDataHandler *handler)
{
    d->handler = handler;
}

ModelDataHandler *FileProxyModel::handler() const
{
    return d->handler;
}

QModelIndex FileProxyModel::rootIndex() const
{
    return createIndex(INT_MAX, 0, (void *)this);
}

QModelIndex FileProxyModel::index(const QUrl &url, int column) const
{
    if (!url.isValid())
        return QModelIndex();

    if (d->fileMap.contains(url)) {
        int row = d->fileList.indexOf(url);
        return createIndex(row, column);
    }

    return QModelIndex();
}

DFMLocalFileInfoPointer FileProxyModel::fileInfo(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->shell->fileInfo(index);

    if (!index.isValid() || index.row() >= d->fileList.count())
        return nullptr;

    return d->fileMap.value(fileUrl(index));
}

QList<QUrl> FileProxyModel::files() const
{
    return d->fileList;
}

QUrl FileProxyModel::fileUrl(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->shell->rootUrl();

    if (!index.isValid() || index.row() >= d->fileList.count())
        return QUrl();

    return d->fileList.at(index.row());
}

void FileProxyModel::refresh(const QModelIndex &parent, bool global, int ms)
{
    if (parent != rootIndex())
        return;

    if (d->refreshTimer.get())
        d->refreshTimer->stop();

    if (ms < 1) {
        d->doRefresh(global);
    } else {
        d->refreshTimer.reset(new QTimer);
        d->refreshTimer->setSingleShot(true);
        connect(d->refreshTimer.get(), &QTimer::timeout, this, [this, global](){
            d->doRefresh(global);
        });

        d->refreshTimer->start(ms);
    }
}

void FileProxyModel::update()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()), 0));
}

QModelIndex FileProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    auto url = fileUrl(proxyIndex);
    if (!url.isValid())
        return QModelIndex();

    return d->shell->index(url);
}

QModelIndex FileProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    auto url = d->shell->fileUrl(sourceIndex);
    if (!url.isValid())
        return QModelIndex();

    return index(url);
}

QModelIndex FileProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || d->fileList.count() <= row)
        return QModelIndex();

    auto url = d->fileList.at(row);
    if (d->fileMap.contains(url))
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex FileProxyModel::parent(const QModelIndex &child) const
{
    if (child != rootIndex() && child.isValid())
        return rootIndex();

    return QModelIndex();
}

int FileProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return d->fileList.count();

    return 0;
}

int FileProxyModel::columnCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return 1;

    return 0;
}

QVariant FileProxyModel::data(const QModelIndex &index, int role) const
{
    QModelIndex sourceIndex = mapToSource(index);
    if (!index.isValid() || !sourceIndex.isValid())
        return QVariant();

    return sourceModel()->data(sourceIndex, role);
}

QMimeData *FileProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << fileUrl(idx);

    data->setUrls(urls);

    // set user id
    data->setData(QString(DFMGLOBAL_NAMESPACE::kMimeDataUserIDKey), QString::number(SysInfoUtils::getUserId()).toLocal8Bit());

    return data;
}

bool FileProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QList<QUrl> urlList = data->urls();
    if (urlList.isEmpty())
        return false;

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = fileUrl(rootIndex());
        qInfo() << "drop file to desktop" << targetFileUrl << "data" << urlList << action;
    } else {
        targetFileUrl = fileUrl(parent);
        qInfo() << "drop file to " << targetFileUrl << "data:" << urlList << action;
    }

    QString errString;
    auto itemInfo =  InfoFactory::create<LocalFileInfo>(targetFileUrl, true, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "create LocalFileInfo error: " << errString << targetFileUrl;
        return false;
    }

    if (itemInfo->isSymLink()) {
        targetFileUrl = itemInfo->symLinkTarget();
    }

    if (DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        // todo(wangcl)
//        FileOperatorProxyIns->dropToTrash(urlList);
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFile(targetFileUrl)) {
        // todo(wangcl)
//        FileOperatorProxyIns->dropToApp(urlList, targetFileUrl.toLocalFile());
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction:
        // todo(wangcl)
//        FileOperatorProxyIns->dropFiles(action, targetFileUrl, urlList);
        break;
    case Qt::LinkAction:
        break;
    default:
        return false;
    }

    return true;
}

void FileProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_UNUSED(sourceModel);
    qWarning() << "forbid setting source model";
    return;
}
