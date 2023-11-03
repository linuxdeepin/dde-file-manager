// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasproxymodel_p.h"
#include "view/operator/fileoperatorproxy.h"
#include "utils/fileutil.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/application/application.h>

#include <dfm-framework/event/event.h>

#include <QMimeData>
#include <QDateTime>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace ddplugin_canvas;

CanvasProxyModelPrivate::CanvasProxyModelPrivate(CanvasProxyModel *qq)
    : QObject(qq), q(qq)
{
    // the hook filter must be first filter.
    modelFilters << QSharedPointer<CanvasModelFilter>(new HookFilter(qq));

    modelFilters << QSharedPointer<CanvasModelFilter>(new HiddenFileFilter(qq));
    modelFilters << QSharedPointer<CanvasModelFilter>(new InnerDesktopAppFilter(qq));
    isNotMixDirAndFile = !Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
}

void CanvasProxyModelPrivate::sourceAboutToBeReset()
{
    q->beginResetModel();
}

void CanvasProxyModelPrivate::sourceReset()
{
    createMapping();
    q->endResetModel();
    qInfo() << "canvas model reseted, file count:" << fileList.count();

    sendLoadReport();
}

void CanvasProxyModelPrivate::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    if ((start < 0) || (end < 0))
        return;

    QList<QUrl> files;
    for (int i = start; i <= end; ++i) {
        auto url = srcModel->fileUrl(srcModel->index(i));

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
    // canvas filter
    bool ignore = renameFilter(oldUrl, newUrl);

    int row = fileList.indexOf(oldUrl);
    if (ignore) {
        if (row >= 0) {
            q->beginRemoveRows(q->rootIndex(), row, row);
            fileList.removeAt(row);
            fileMap.remove(oldUrl);
            q->endRemoveRows();
        }
        return;
    }

    auto newInfo = srcModel->fileInfo(srcModel->index(newUrl));
    if (Q_LIKELY(row < 0)) {   // no old data, need to insert
        if (!fileMap.contains(newUrl)) {   // insert it if it does not exist.
            row = fileList.count();
            q->beginInsertRows(q->rootIndex(), row, row);
            fileList.append(newUrl);
            fileMap.insert(newUrl, newInfo);
            q->endInsertRows();
            return;
        }
    } else {
        // e.g. a mv to b(b is existed)
        if (fileMap.contains(newUrl)) {
            //! treat as removing if newurl is existed in canvas.
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

    //! warrning: the root url and FileInfo::url must be like file://
    QDir dir(q->rootUrl().toString());
    QList<QPair<QString, QUrl>> mainDesktop = { { dir.filePath("dde-home.desktop"), QUrl() },
                                                { dir.filePath("dde-trash.desktop"), QUrl() },
                                                { dir.filePath("dde-computer.desktop"), QUrl() } };
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

void CanvasProxyModelPrivate::sendLoadReport()
{
    static bool reportedFirstLoad { false };
    if (Q_LIKELY(reportedFirstLoad))
        return;

    QVariantMap data {};
    data.insert("filesCount", fileList.count());
    data.insert("time", QDateTime::currentDateTime().toString());
    dpfSignalDispatcher->publish("ddplugin_canvas", "signal_ReportLog_LoadFilesFinish",
                                 QString(DFMGLOBAL_NAMESPACE::DataPersistence::kDesktopLoadFilesTime),
                                 QVariant(data));
    reportedFirstLoad = true;
}

bool CanvasProxyModelPrivate::insertFilter(const QUrl &url)
{
    bool ret = std::any_of(modelFilters.begin(), modelFilters.end(),
                           [&url](const QSharedPointer<CanvasModelFilter> &filter) {
                               return filter->insertFilter(url);
                           });

    return ret;
}

bool CanvasProxyModelPrivate::resetFilter(QList<QUrl> &urls)
{
    bool ret = std::any_of(modelFilters.begin(), modelFilters.end(),
                           [&urls](const QSharedPointer<CanvasModelFilter> &filter) {
                               return filter->resetFilter(urls);
                           });

    return ret;
}

bool CanvasProxyModelPrivate::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    // these filters is like Notifier.
    // so it will don't interrupt when some one return true.
    bool ret = false;
    auto unused = std::all_of(modelFilters.begin(), modelFilters.end(),
                              [&ret, &url, &roles](const QSharedPointer<CanvasModelFilter> &filter) {
                                  ret |= filter->updateFilter(url, roles);
                                  return true;
                              });
    Q_UNUSED(unused);

    return ret;
}

bool CanvasProxyModelPrivate::removeFilter(const QUrl &url)
{
    bool ret = false;
    auto unused = std::all_of(modelFilters.begin(), modelFilters.end(),
                              [&ret, &url](const QSharedPointer<CanvasModelFilter> &filter) {
                                  ret |= filter->removeFilter(url);
                                  return true;
                              });
    Q_UNUSED(unused);
    return ret;
}

bool CanvasProxyModelPrivate::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ret = false;
    auto unused = std::all_of(modelFilters.begin(), modelFilters.end(),
                              [&ret, &oldUrl, &newUrl](const QSharedPointer<CanvasModelFilter> &filter) {
                                  ret |= filter->renameFilter(oldUrl, newUrl);
                                  return true;
                              });
    Q_UNUSED(unused);

    return ret;
}

bool CanvasProxyModelPrivate::lessThan(const QUrl &left, const QUrl &right) const
{
    QModelIndex leftIdx = q->index(left);
    QModelIndex rightIdx = q->index(right);

    if (!leftIdx.isValid() || !rightIdx.isValid())
        return false;

    FileInfoPointer leftInfo = fileMap.value(left);
    FileInfoPointer rightInfo = fileMap.value(right);

    // The folder is fixed in the front position
    if (isNotMixDirAndFile) {
        if (leftInfo->isAttributes(OptInfoType::kIsDir)) {
            if (!rightInfo->isAttributes(OptInfoType::kIsDir))
                return true;
        } else {
            if (rightInfo->isAttributes(OptInfoType::kIsDir))
                return false;
        }
    }

    QVariant leftData = q->data(leftIdx, fileSortRole);
    QVariant rightData = q->data(rightIdx, fileSortRole);

    // When the selected sort attribute value is the same, sort by file name
    auto compareByName = [this, leftIdx, rightIdx]() {
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

    std::stable_sort(files.begin(), files.end(), [this](const QUrl &left, const QUrl &right) {
        return lessThan(left, right);
    });

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

    // canvas filter
    resetFilter(urls);

    // sort
    QMap<QUrl, FileInfoPointer> maps;
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
            maps.insert(url, fileMap.value(url));
    }

    fileList = urls;
    fileMap = maps;
}

QModelIndexList CanvasProxyModelPrivate::indexs() const
{
    QModelIndexList results;
    for (int i = 0; i < q->rowCount(q->rootIndex()); i++) {
        QModelIndex childIndex = q->index(i);
        results << childIndex;
    }
    return results;
}

QModelIndexList CanvasProxyModelPrivate::indexs(const QList<QUrl> &files) const
{
    QModelIndexList idxs;
    for (const QUrl &toUrl : files)
        idxs << q->index(toUrl);
    return idxs;
}

bool CanvasProxyModelPrivate::doSort(QList<QUrl> &files) const
{
    if (files.isEmpty())
        return true;

    if (hookIfs && hookIfs->sortData(fileSortRole, fileSortOrder, &files)) {
        qDebug() << "using extend sort";
        return true;
    }

    // standard sort function
    standardSort(files);

    // advanced sort for special case.
    specialSort(files);
    return true;
}

void CanvasProxyModelPrivate::doRefresh(bool global, bool refreshFile)
{
    if (global) {
        srcModel->refresh(srcModel->rootIndex());
    } else {
        // refresh all file info
        if (refreshFile) {
            // do not emit data changed signal, just refresh file info.
            QSignalBlocker blocker(srcModel);
            srcModel->update();
        }

        // reset model
        sourceAboutToBeReset();
        sourceReset();

        // refresh file info in canvas model
        {
            // do not emit data changed signal, just refresh file info.
            QSignalBlocker blocker(q);
            q->update();
        }
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
        if (hookIfs && hookIfs->dataChanged(url, roles)) {
            qWarning() << "invalid module: dataChanged returns true.";
        }

        // canvas filter
        updateFilter(url, roles);

        auto cur = q->index(url);
        if (cur.isValid())
            idxs << cur;
    }

    if (idxs.isEmpty())
        return;

    // AscendingOrder
    std::stable_sort(idxs.begin(), idxs.end(), [](const QModelIndex &t1, const QModelIndex &t2) {
        return t1.row() < t2.row();
    });

    emit q->dataChanged(idxs.first(), idxs.last(), roles);
}

CanvasProxyModel::CanvasProxyModel(QObject *parent)
    : QAbstractProxyModel(parent), d(new CanvasProxyModelPrivate(this))
{
}

QModelIndex CanvasProxyModel::rootIndex() const
{
    return createIndex(INT_MAX, 0, reinterpret_cast<void *>(const_cast<CanvasProxyModel *>(this)));
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

FileInfoPointer CanvasProxyModel::fileInfo(const QModelIndex &index) const
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

void CanvasProxyModel::setModelHook(ModelHookInterface *ext)
{
    d->hookIfs = ext;
}

ModelHookInterface *CanvasProxyModel::modelHook() const
{
    return d->hookIfs;
}

void CanvasProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel())
        return;

    FileInfoModel *fileModel = dynamic_cast<FileInfoModel *>(model);
    Q_ASSERT(fileModel);

    d->srcModel = fileModel;
    beginResetModel();

    if (auto oldModel = sourceModel()) {
        oldModel->disconnect(this);
        oldModel->disconnect(d);
    }

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
    if (d->hookIfs) {
        auto url = fileUrl(index);
        QVariant var;
        if (d->hookIfs->modelData(url, itemRole, &var))
            return var;
    }

    return d->srcModel->data(sourceIndex, itemRole);
}

QStringList CanvasProxyModel::mimeTypes() const
{
    auto list = QAbstractProxyModel::mimeTypes();

    if (d->hookIfs) {
        d->hookIfs->mimeTypes(&list);
        qDebug() << "using extend mimeTypes." << list;
    }

    return list;
}

QMimeData *CanvasProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimedt = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << fileUrl(idx);

    if (d->hookIfs && d->hookIfs->mimeData(urls, mimedt)) {
        qDebug() << "using extend mimeData.";
    } else {
        mimedt->setUrls(urls);
    }

    // set user id
    SysInfoUtils::setMimeDataUserId(mimedt);

    return mimedt;
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

    if (itemInfo->isAttributes(OptInfoType::kIsSymLink)) {
        targetFileUrl = QUrl::fromLocalFile(itemInfo->pathOf(PathInfoType::kSymLinkTarget));
    }

    if (d->hookIfs && d->hookIfs->dropMimeData(data, targetFileUrl, action)) {
        qInfo() << "dropMimeData by extend module.";
        return true;
    }

    // treeveiew drop urls
    QList<QUrl> treeSelectUrl;
    if (data->formats().contains(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey)) {
        auto treeUrlsStr = QString(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey));
        auto treeUrlss = treeUrlsStr.split("\n");
        for (const auto &url : treeUrlss) {
            if (url.isEmpty())
                continue;
            treeSelectUrl.append(QUrl(url));
        }
    }
    if (DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        FileOperatorProxyIns->dropToTrash(treeSelectUrl.isEmpty() ? urlList : treeSelectUrl);
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        // nothing to do.
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFile(targetFileUrl)) {
        FileOperatorProxyIns->dropToApp(urlList, targetFileUrl.toLocalFile());
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction:
        if (urlList.count() > 0)
            FileOperatorProxyIns->dropFiles(action, targetFileUrl, treeSelectUrl.isEmpty() ? urlList : treeSelectUrl);
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

    QMap<QUrl, FileInfoPointer> tempFileMap;
    QList<QUrl> orderFiles = d->fileList;
    if (!d->doSort(orderFiles))
        return false;

    for (const QUrl &url : orderFiles)
        tempFileMap.insert(url, d->srcModel->fileInfo(d->srcModel->index(url)));

    layoutAboutToBeChanged();
    {
        // get the indexs and urls before sorting.
        QModelIndexList from = d->indexs();
        auto fromUlrs = d->fileList;

        d->fileList = orderFiles;
        d->fileMap = tempFileMap;

        // get the indexs of fromUlrs after sorting
        QModelIndexList to = d->indexs(fromUlrs);
        changePersistentIndexList(from, to);
    }
    layoutChanged();

    return true;
}

void CanvasProxyModel::update()
{
    qInfo() << "update file info in model." << d->fileMap.size();
    if (d->fileMap.isEmpty())
        return;

    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()) - 1, 0));
}

void CanvasProxyModel::refresh(const QModelIndex &parent, bool global, int ms, bool refreshFile)
{
    d->isNotMixDirAndFile = !Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();

    if (parent != rootIndex())
        return;

    if (d->refreshTimer.get())
        d->refreshTimer->stop();

    if (ms < 1) {
        d->doRefresh(global, refreshFile);
    } else {
        d->refreshTimer.reset(new QTimer);
        d->refreshTimer->setSingleShot(true);
        connect(d->refreshTimer.get(), &QTimer::timeout, this, [this, global, refreshFile]() {
            d->doRefresh(global, refreshFile);
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

    if (d->hookIfs)
        d->hookIfs->hiddenFlagChanged(show);
}

bool CanvasProxyModel::fetch(const QUrl &url)
{
    if (d->fileMap.contains(url))
        return true;

    QModelIndex idx = d->srcModel->index(url);
    if (!idx.isValid())
        return false;

    auto info = d->srcModel->fileInfo(idx);
    if (info) {
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
