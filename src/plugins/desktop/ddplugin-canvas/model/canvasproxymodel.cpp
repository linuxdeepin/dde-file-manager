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
#include "canvasproxymodel_p.h"
#include "view/operator/fileoperatorproxy.h"
#include "utils/fileutil.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <QMimeData>
#include <QDateTime>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

CanvasProxyModelPrivate::CanvasProxyModelPrivate(CanvasProxyModel *qq)
    : QObject(qq)
    , q(qq)
{
    modelFilters << QSharedPointer<CanvasModelFilter>(new HiddenFileFilter(qq));
    modelFilters << QSharedPointer<CanvasModelFilter>(new InnerDesktopAppFilter(qq));
}

void CanvasProxyModelPrivate::sourceAboutToBeReset()
{
    q->beginResetModel();
}

void CanvasProxyModelPrivate::sourceReset()
{
    createMapping();
    q->endResetModel();
}

void CanvasProxyModelPrivate::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    if ((start < 0) || (end < 0))
        return;

    QList<QUrl> files;
    for (int i = start; i <= end; ++i) {
        auto url = srcModel->fileUrl(srcModel->index(i));
        if (extend && extend->dataInserted(url)) {
            qDebug() << "filter by extend module:" << url;
            continue;
        }

        // canvas filter
        if (insertFilter(url))
            continue;

        if (!fileMap.contains(url))
            files << url;
    }

    if (files.isEmpty())
        return;

    int row = fileList.count();
    q->beginInsertRows(q->rootIndex(), row, row + files.count() - 1);

    fileList.append(files);
    for (const QUrl &url : files)
        fileMap.insert(url, srcModel->fileInfo(srcModel->index(url)));

    q->endInsertRows();
}

void CanvasProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    Q_UNUSED(sourceParent)
    if ((start < 0) || (end < 0))
        return;

    QList<QUrl> files;
    for (int i = start; i <= end; ++i) {
        auto url = srcModel->fileUrl(srcModel->index(i));
        if (extend && extend->dataRemoved(url)) {
            qWarning() << "invalid module: dataRemoved returns true.";
        }

        // canvas filter
        removeFilter(url);

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

void CanvasProxyModelPrivate::sourceDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ignore = false;
    if (extend) {
        if (ignore = extend->dataRenamed(oldUrl, newUrl))
            qDebug() << "dataRenamed: ignore target" << newUrl << "old:" << oldUrl;
    }

    // canvas filter
    ignore = ignore || renameFilter(oldUrl, newUrl);

    int row = fileList.indexOf(oldUrl);
    if (ignore) {
        if (row >= 0) {
            // see as remove
            removeFilter(oldUrl);

            q->beginRemoveRows(q->rootIndex(), row, row);
            fileList.removeAt(row);
            fileMap.remove(oldUrl);
            q->endRemoveRows();
        }
        return;
    }

    auto newInfo = srcModel->fileInfo(srcModel->index(newUrl));
    if (Q_LIKELY(row < 0)) { // no old data, need to insert
        if (!fileMap.contains(newUrl)) { // insert it if it does not exist.
            row = fileList.count();
            q->beginInsertRows(q->rootIndex(), row, row);
            fileList.append(newUrl);
            fileMap.insert(newUrl, newInfo);
            q->endInsertRows();
            return;
        }
    } else {
        if (fileList.contains(newUrl)) {
            // e.g. a mv to b(b is existed)
            q->beginRemoveRows(q->rootIndex(), row, row);
            fileList.removeAt(row);
            q->endRemoveRows();

            row = fileList.indexOf(newUrl);
        } else {
            fileList.replace(row, newUrl);
        }

        fileMap.remove(oldUrl);
        fileMap.insert(newUrl, newInfo);
        emit q->dataReplaced(oldUrl, newUrl);

        auto index = q->index(row);
        emit q->dataChanged(index, index);
    }
}

void CanvasProxyModelPrivate::specialSort(QList<QUrl> &files) const
{
    if (fileSortRole == ItemRoles::kItemFileMimeTypeRole)
        sortMainDesktopFile(files, fileSortOrder);
}

void CanvasProxyModelPrivate::sortMainDesktopFile(QList<QUrl> &files, Qt::SortOrder order) const
{
    // let the main desktop files always on front or back.

    //! warrning: the root url and LocalFileInfo::url must be like file://
    QDir dir(q->rootUrl().toString());
    QList<QPair<QString, QUrl>> mainDesktop = {{dir.filePath("dde-home.desktop"), QUrl()},
        {dir.filePath("dde-trash.desktop"), QUrl()},
        {dir.filePath("dde-computer.desktop"), QUrl()}
    };
    auto list = files;
    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        for (const QUrl &url : list)
        if (url.toString() == it->first) {
            it->second = url;
            files.removeOne(url);
        }
    }

    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        if (it->second.isValid()) {
            if (order == Qt::AscendingOrder)
                files.push_front(it->second);
            else
                files.push_back(it->second);
        }
    }
}

bool CanvasProxyModelPrivate::insertFilter(const QUrl &url)
{
    for (const auto &filter : modelFilters)
        if (filter->insertFilter(url))
            return true;
    return false;
}

bool CanvasProxyModelPrivate::resetFilter(QList<QUrl> &urls)
{
    for (const auto &filter : modelFilters)
        if (filter->resetFilter(urls))
            return true;
    return false;
}

bool CanvasProxyModelPrivate::updateFilter(const QUrl &url)
{
    // these filters is like Notifier.
    // so it will don't interrupt when some one return true.
    bool ret = false;
    for (const auto &filter : modelFilters)
        ret |= filter->updateFilter(url);

    return ret;
}

bool CanvasProxyModelPrivate::removeFilter(const QUrl &url)
{
    bool ret = false;
    for (const auto &filter : modelFilters)
        ret |= filter->removeFilter(url);

    return ret;
}

bool CanvasProxyModelPrivate::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ret = false;
    for (const auto &filter : modelFilters)
        ret |= filter->renameFilter(oldUrl, newUrl);

    return ret;
}

bool CanvasProxyModelPrivate::lessThan(const QUrl &left, const QUrl &right) const
{
    QModelIndex leftIdx = q->index(left);
    QModelIndex rightIdx = q->index(right);

    if (!leftIdx.isValid() || !rightIdx.isValid())
        return false;

    DFMLocalFileInfoPointer leftInfo = q->fileInfo(leftIdx);
    DFMLocalFileInfoPointer rightInfo = q->fileInfo(rightIdx);

    // The folder is fixed in the front position
    if (leftInfo->isDir()) {
        if (!rightInfo->isDir())
            return true;
    } else {
        if (rightInfo->isDir())
            return false;
    }

    QVariant leftData = q->data(leftIdx, fileSortRole);
    QVariant rightData = q->data(rightIdx, fileSortRole);

    // When the selected sort attribute value is the same, sort by file name
    auto compareByName = [this, leftIdx, rightIdx](){
        QString leftName = q->data(leftIdx, kItemFileDisplayNameRole).toString();
        QString rightName = q->data(rightIdx, kItemFileDisplayNameRole).toString();
        return FileUtils::compareString(leftName, rightName, fileSortOrder);
    };

    switch (fileSortRole) {
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
    case kItemFileDisplayNameRole: {
        QString leftString = leftData.toString();
        QString rightString = rightData.toString();
        return leftString == rightString ? compareByName() : FileUtils::compareString(leftString, rightString, fileSortOrder);
    }
    case kItemFileSizeRole: {
        qint64 leftSize = leftData.toLongLong();
        qint64 rightSize = rightData.toLongLong();
        return leftSize == rightSize ? compareByName() : ((fileSortOrder == Qt::DescendingOrder) ^ (leftSize < rightSize)) == 0x01;
    }
    default:
        return false;
    }
}

void CanvasProxyModelPrivate::standardSort(QList<QUrl> &files) const
{
    if (files.isEmpty())
        return;

    std::sort(files.begin(), files.end(), [this](const QUrl &left, const QUrl &right) {
        return lessThan(left, right);
    });

    // advanced sort for special case.
    specialSort(files);
    return;
}

void CanvasProxyModelPrivate::clearMapping()
{
    fileList.clear();
    fileMap.clear();
}

void CanvasProxyModelPrivate::createMapping()
{
    if (!srcModel)
        return;

    auto urls = srcModel->files();
    if (extend && extend->dataRested(&urls)) {
        qWarning() << "invalid module: dataRested returns true.";
    }

    // canvas filter
    resetFilter(urls);

    // sort
    QMap<QUrl, DFMLocalFileInfoPointer> maps;
    for (const QUrl &url : urls)
        maps.insert(url, srcModel->fileInfo(srcModel->index(url)));

    // set unsorted files into model to enable create module index that doSort will used.
    fileList = urls;
    fileMap = maps;

    doSort(urls);

    // update fileinfo list
    {
        maps.clear();
        for (const QUrl &url : urls)
            maps.insert(url, srcModel->fileInfo(srcModel->index(url)));
    }


    fileList = urls;
    fileMap = maps;
}

QModelIndexList CanvasProxyModelPrivate::indexs() const
{
    QModelIndexList results;
    for (int i = 0; i < q->rowCount(); i++) {
        QModelIndex childIndex = q->index(i);
        results << childIndex;
    }
    return results;
}

bool CanvasProxyModelPrivate::doSort(QList<QUrl> &files) const
{
    if (files.isEmpty())
        return true;

    if (extend && extend->sortData(fileSortRole, fileSortOrder, &files)) {
        qDebug() << "using extend sort";
        return true;
    }

    // standard sort function
    standardSort(files);
    return true;
}

void CanvasProxyModelPrivate::doRefresh(bool global)
{
    if (global) {
        srcModel->refresh(srcModel->rootIndex());
    } else {
        sourceAboutToBeReset();
        sourceReset();
        q->update();
    }
}

void CanvasProxyModelPrivate::sourceDataChanged(const QModelIndex &sourceTopleft, const QModelIndex &sourceBottomright, const QVector<int> &roles)
{
    if (!sourceTopleft.isValid() || !sourceBottomright.isValid())
        return;

    int begin = qMin(sourceTopleft.row(), sourceBottomright.row());
    int end = qMax(sourceTopleft.row(), sourceBottomright.row());

    QList<QModelIndex> idxs;
    // find items in this model
    for (int i = begin; i <= end; ++i) {
        auto url = srcModel->fileUrl(srcModel->index(i));
        if (extend && extend->dataChanged(url)) {
            qWarning() << "invalid module: dataChanged returns true.";
        }

        // canvas filter
        updateFilter(url);

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

CanvasProxyModel::CanvasProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new CanvasProxyModelPrivate(this))
{

}

QModelIndex CanvasProxyModel::rootIndex() const
{
    return createIndex((quintptr)this, 0, (void *)this);
}

QModelIndex CanvasProxyModel::index(const QUrl &url, int column) const
{
    if (!url.isValid())
        return QModelIndex();

    if (d->fileMap.contains(url)) {
        int row = d->fileList.indexOf(url);
        return createIndex(row, column);
    }

    return QModelIndex();
}

DFMLocalFileInfoPointer CanvasProxyModel::fileInfo(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->srcModel->fileInfo(mapToSource(index));

    if (index.row() < 0 || index.row() >= d->fileList.count())
        return nullptr;

    return d->fileMap.value(d->fileList.at(index.row()));
}

QUrl CanvasProxyModel::fileUrl(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->srcModel->rootUrl();

    if (!index.isValid() || index.row() >= d->fileList.count())
        return QUrl();

    return d->fileList.at(index.row());
}

QList<QUrl> CanvasProxyModel::files() const
{
    return d->fileList;
}

bool CanvasProxyModel::showHiddenFiles() const
{
    return d->filters & QDir::Hidden;
}

Qt::SortOrder CanvasProxyModel::sortOrder() const
{
    return d->fileSortOrder;
}

void CanvasProxyModel::setSortOrder(const Qt::SortOrder &order)
{
    d->fileSortOrder = order;
}

int CanvasProxyModel::sortRole() const
{
    return d->fileSortRole;
}

void CanvasProxyModel::setSortRole(int role, Qt::SortOrder order)
{
    d->fileSortRole = role;
    d->fileSortOrder = order;
}

void CanvasProxyModel::setModelExtend(ModelExtendInterface *ext)
{
    d->extend = ext;
}

ModelExtendInterface *CanvasProxyModel::modelExtend() const
{
    return d->extend;
}

void CanvasProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel())
        return;

    FileInfoModel *fileModel = dynamic_cast<FileInfoModel *>(model);
    Q_ASSERT(fileModel);

    d->srcModel = fileModel;
    beginResetModel();

    if (auto oldModel = sourceModel())
        oldModel->disconnect(this);

    d->clearMapping();

    QAbstractProxyModel::setSourceModel(model);

    // for rename
    connect(fileModel, &FileInfoModel::dataReplaced,
            d, &CanvasProxyModelPrivate::sourceDataRenamed);

    connect(model, &QAbstractItemModel::dataChanged,
            d, &CanvasProxyModelPrivate::sourceDataChanged);

    connect(model, &QAbstractItemModel::rowsInserted,
            d, &CanvasProxyModelPrivate::sourceRowsInserted);

    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,
            d, &CanvasProxyModelPrivate::sourceRowsAboutToBeRemoved);

    connect(model, &QAbstractItemModel::modelAboutToBeReset,
            d, &CanvasProxyModelPrivate::sourceAboutToBeReset);

    connect(model, &QAbstractItemModel::modelReset,
            d, &CanvasProxyModelPrivate::sourceReset);

    endResetModel();

    //! the sgnals listed below is useless.
    //! The original model did not involve sorting

//    connect(model, &QAbstractItemModel::columnsAboutToBeRemoved),
//            d, &CanvasProxyModelPrivate::sourceColumnsAboutToBeRemoved);
//    connect(model, &QAbstractItemModel::columnsRemoved),
//            d, &CanvasProxyModelPrivate::sourceColumnsRemoved);

//    connect(model, &QAbstractItemModel::columnsAboutToBeInserted,
//            d, &CanvasProxyModelPrivate::sourceColumnsAboutToBeInserted);
//    connect(model, &QAbstractItemModel::rowsRemoved,
//            d, &CanvasProxyModelPrivate::sourceRowsRemoved);

//    connect(model, &QAbstractItemModel::columnsInserted,
//            d, &CanvasProxyModelPrivate::sourceColumnsInserted);

//    connect(model, &QAbstractItemModel::rowsAboutToBeInserted,
//            d, &CanvasProxyModelPrivate::sourceRowsAboutToBeInserted);

//    connect(model, &QAbstractItemModel::rowsAboutToBeMoved,
//            d, &CanvasProxyModelPrivate::sourceRowsAboutToBeMoved);

//    connect(model, &QAbstractItemModel::rowsMoved,
//            d, &CanvasProxyModelPrivate::sourceRowsMoved);

//    connect(model, &QAbstractItemModel::columnsAboutToBeMoved,
//            d, &CanvasProxyModelPrivate::sourceColumnsAboutToBeMoved);

//    connect(model, &QAbstractItemModel::columnsMoved,
//            d, &CanvasProxyModelPrivate::sourceColumnsMoved);

//    connect(model, &QAbstractItemModel::layoutAboutToBeChanged,
//            d, &CanvasProxyModelPrivate::sourceLayoutAboutToBeChanged);

//    connect(model, &QAbstractItemModel::layoutChanged,
//            d, &CanvasProxyModelPrivate::sourceLayoutChanged);

}

QModelIndex CanvasProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    auto url = fileUrl(proxyIndex);
    if (!url.isValid())
        return QModelIndex();

    return d->srcModel->index(url);
}

QModelIndex CanvasProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    auto url = d->srcModel->fileUrl(sourceIndex);
    if (!url.isValid())
        return QModelIndex();

    return index(url);
}

QModelIndex CanvasProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || d->fileList.count() <= row)
        return QModelIndex();

    auto url = d->fileList.at(row);
    if (d->fileMap.contains(url))
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex CanvasProxyModel::parent(const QModelIndex &child) const
{
    if (child != rootIndex() && child.isValid())
        return rootIndex();

    return QModelIndex();
}

int CanvasProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return d->fileList.count();

    return 0;
}

int CanvasProxyModel::columnCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return 1;

    return 0;
}

QVariant CanvasProxyModel::data(const QModelIndex &index, int itemRole) const
{
    QModelIndex sourceIndex = mapToSource(index);
    if (!index.isValid() || !sourceIndex.isValid())
        return QVariant();

    // canvas extend
    if (d->extend) {
        auto url = fileUrl(index);
        QVariant var;
        if (d->extend->modelData(url, itemRole, &var))
            return var;
    }

    return d->srcModel->data(sourceIndex, itemRole);
}

QStringList CanvasProxyModel::mimeTypes() const
{
    auto list = QAbstractProxyModel::mimeTypes();

    if (d->extend ) {
        d->extend->mimeTypes(&list);
        qDebug() << "using extend mimeTypes." << list;
    }

    return list;
}

QMimeData *CanvasProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << fileUrl(idx);

    if (d->extend && d->extend->mimeData(urls, data)) {
        qDebug() << "using extend mimeData.";
    } else {
        data->setUrls(urls);
    }

    // set user id
    data->setData(QString(DFMGLOBAL_NAMESPACE::kMimeDataUserIDKey), QString::number(SysInfoUtils::getUserId()).toLocal8Bit());

    return data;
}

bool CanvasProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QList<QUrl> urlList = data->urls();
    if (urlList.isEmpty())
        return false;

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = d->srcModel->rootUrl();
        qInfo() << "drop file to desktop" << targetFileUrl << "data" << urlList << action;
    } else {
        targetFileUrl = fileUrl(parent);
        qInfo() << "drop file to " << targetFileUrl << "data:" << urlList << action;
    }

    auto itemInfo = FileCreator->createFileInfo(targetFileUrl);
    if (Q_UNLIKELY(!itemInfo))
        return false;

    if (itemInfo->isSymLink()) {
        targetFileUrl = itemInfo->symLinkTarget();
    }

    if (d->extend && d->extend->dropMimeData(data, targetFileUrl, action)) {
        qDebug() << "droped by extend module.";
        return true;
    }

    if (itemInfo->canDragCompress()
            && !itemInfo->isDir()) { // don't compress is itemInfo is dir.
        qDebug() << "drop to append compress files.";
        return false;//FileUtils::appendCompress(toUrl, urlList);    // todo Compress
    }

    if (DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        FileOperatorProxyIns->dropToTrash(urlList);
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFile(targetFileUrl)) {
        FileOperatorProxyIns->dropToApp(urlList, targetFileUrl.toLocalFile());
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction:
        FileOperatorProxyIns->dropFiles(action, targetFileUrl, urlList);
        break;
    case Qt::LinkAction:
        break;
    default:
        return false;
    }

    return true;
}

bool CanvasProxyModel::sort()
{
    if (d->fileList.isEmpty())
        return true;

    QMap<QUrl, DFMLocalFileInfoPointer> tempFileMap;
    QList<QUrl> orderFiles = d->fileList;
    if (!d->doSort(orderFiles))
        return false;

    for (const QUrl &url : orderFiles)
        tempFileMap.insert(url, d->srcModel->fileInfo(d->srcModel->index(url)));

    layoutAboutToBeChanged();
    {
        QModelIndexList from = d->indexs();
        d->fileList = orderFiles;
        d->fileMap = tempFileMap;
        QModelIndexList to = d->indexs();
        changePersistentIndexList(from, to);
    }
    layoutChanged();

    return true;
}

void CanvasProxyModel::update()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()), 0));
}

void CanvasProxyModel::refresh(const QModelIndex &parent, bool global, int ms)
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

void CanvasProxyModel::setShowHiddenFiles(bool show)
{
    if (show)
        d->filters |= QDir::Hidden;
    else
        d->filters &= ~QDir::Hidden;
}

bool CanvasProxyModel::fetch(const QUrl &url)
{
    if (d->fileMap.contains(url))
        return true;

    QModelIndex index = d->srcModel->index(url);
    if (!index.isValid())
        return false;

    auto info = d->srcModel->fileInfo(index);
    if (info) {
        if (d->extend && d->extend->dataInserted(url)) {
            qDebug() << "filter by extend module. can not add" << url;
            return false;
        }

        // canvas filter
        if (d->insertFilter(url)) {
            qDebug() << "filter it, don't add" << url;
            return false;
        }

        int row = d->fileList.count();
        beginInsertRows(rootIndex(), row, row);

        d->fileList.append(url);
        d->fileMap.insert(url, info);

        endInsertRows();
        return true;
    }

    qDebug() << "fail to add: no such file" << url;
    return false;
}

bool CanvasProxyModel::take(const QUrl &url)
{
    if (!d->fileMap.contains(url))
        return true;

    if (d->extend && d->extend->dataRemoved(url)) {
        qWarning() << "invalid module: dataRemoved returns true.";
    }

    // canvas filter
    d->removeFilter(url);

    int row = d->fileList.indexOf(url);
    if (Q_UNLIKELY(row < 0)) {
        qCritical() << "invaild index of" << url;
        return false;
    }

    beginRemoveRows(rootIndex(), row, row);
    d->fileList.removeAt(row);
    d->fileMap.remove(url);
    endRemoveRows();
    return true;
}
