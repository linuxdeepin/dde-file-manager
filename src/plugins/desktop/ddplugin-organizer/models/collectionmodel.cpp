// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionmodel_p.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/fileinfomodelshell.h"
#include "modeldatahandler.h"
#include "utils/fileoperator.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

inline constexpr char kDdeDesktopOrganizer[] { "dde-desktop-organizer" };

CollectionModelPrivate::CollectionModelPrivate(CollectionModel *qq)
    : QObject(qq), q(qq)
{
}

CollectionModelPrivate::~CollectionModelPrivate()
{
}

void CollectionModelPrivate::reset()
{
    fileList.clear();
    fileMap.clear();

    auto model = q->sourceModel();
    if (!model) {
        fmWarning() << "invaild source model.";
        return;
    }

    // for rename
    connect(shell, &FileInfoModelShell::dataReplaced,
            this, &CollectionModelPrivate::sourceDataRenamed);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &CollectionModelPrivate::sourceDataChanged);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &CollectionModelPrivate::sourceRowsInserted);

    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &CollectionModelPrivate::sourceRowsAboutToBeRemoved);

    connect(model, &QAbstractItemModel::modelAboutToBeReset,
            this, &CollectionModelPrivate::sourceAboutToBeReset);

    connect(model, &QAbstractItemModel::modelReset,
            this, &CollectionModelPrivate::sourceReset);
}

void CollectionModelPrivate::clearMapping()
{
    fileList.clear();
    fileMap.clear();
}

void CollectionModelPrivate::createMapping()
{
    auto source = q->sourceModel();
    if (!source || !shell)
        return;

    if (!handler) {
        fmWarning() << "no handler: clear all datas";
        clearMapping();
        return;
    }

    fileList = handler->acceptReset(shell->files());
    QMap<QUrl, FileInfoPointer> maps;
    for (const QUrl &url : fileList)
        maps.insert(url, shell->fileInfo(shell->index(url)));

    fileMap = maps;
}

void CollectionModelPrivate::sourceDataChanged(const QModelIndex &sourceTopleft, const QModelIndex &sourceBottomright, const QVector<int> &roles)
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

        if (handler && handler->acceptUpdate(url, roles)) {
            if (!fileList.contains(url)) {
                fileList.append(url);
                fileMap.insert(url, shell->fileInfo(q->sourceModel()->index(i, 0)));
            }
        }

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

void CollectionModelPrivate::sourceAboutToBeReset()
{
    q->beginResetModel();
}

void CollectionModelPrivate::sourceReset()
{
    createMapping();
    q->endResetModel();
}

void CollectionModelPrivate::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    Q_UNUSED(sourceParent)
    if (!handler) {
        fmWarning() << "no handler to insert";
        return;
    }

    if ((start < 0) || (end < 0)) {
        fmWarning() << "Invalid insertion range:" << start << "to" << end;
        return;
    }

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

void CollectionModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    Q_UNUSED(sourceParent)
    if ((start < 0) || (end < 0)) {
        fmWarning() << "Invalid removal range:" << start << "to" << end;
        return;
    }

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

void CollectionModelPrivate::sourceDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    int row = fileList.indexOf(oldUrl);
    auto newInfo = shell->fileInfo(shell->index(newUrl));
    bool accept = false;
    if (handler)
        accept = handler->acceptRename(oldUrl, newUrl);
    else
        fmWarning() << "no handler to insert reamed file.";

    if (row < 0) {   // no old data, need to insert
        if (!fileMap.contains(newUrl) && accept) {   // insert it if accept.
            row = fileList.count();
            q->beginInsertRows(q->rootIndex(), row, row);
            fileList.append(newUrl);
            fileMap.insert(newUrl, newInfo);
            q->endInsertRows();
            fmDebug() << "Inserted renamed file as new item";
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

void CollectionModelPrivate::doRefresh(bool global, bool file)
{
    if (global) {
        shell->refresh(shell->rootIndex());
    } else {
        if (file) {
            // do not emit data changed signal, just refresh file info.
            QSignalBlocker blocker(q);
            q->update();
        }

        sourceAboutToBeReset();
        sourceReset();
    }
}

CollectionModel::CollectionModel(QObject *parent)
    : QAbstractProxyModel(parent), d(new CollectionModelPrivate(this))
{
}

CollectionModel::~CollectionModel()
{
    delete d;
    d = nullptr;
}

void CollectionModel::setModelShell(FileInfoModelShell *shell)
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

FileInfoModelShell *CollectionModel::modelShell() const
{
    return d->shell;
}

void CollectionModel::setHandler(ModelDataHandler *handler)
{
    d->handler = handler;
}

ModelDataHandler *CollectionModel::handler() const
{
    return d->handler;
}

QUrl CollectionModel::rootUrl() const
{
    return d->shell->rootUrl();
}

QModelIndex CollectionModel::rootIndex() const
{
    return createIndex(INT_MAX, 0, static_cast<void *>(const_cast<CollectionModel *>(this)));
}

QModelIndex CollectionModel::index(const QUrl &url, int column) const
{
    if (!url.isValid())
        return QModelIndex();

    if (d->fileMap.contains(url)) {
        int row = d->fileList.indexOf(url);
        return createIndex(row, column);
    }

    return QModelIndex();
}

FileInfoPointer CollectionModel::fileInfo(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->shell->fileInfo(index);

    if (!index.isValid() || index.row() >= d->fileList.count())
        return nullptr;

    return d->fileMap.value(fileUrl(index));
}

QList<QUrl> CollectionModel::files() const
{
    return d->fileList;
}

QUrl CollectionModel::fileUrl(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->shell->rootUrl();

    if (!index.isValid() || index.row() >= d->fileList.count())
        return QUrl();

    return d->fileList.at(index.row());
}

void CollectionModel::refresh(const QModelIndex &parent, bool global, int ms, bool file)
{
    if (parent != rootIndex())
        return;

    if (d->refreshTimer.get())
        d->refreshTimer->stop();

    if (ms < 1) {
        d->doRefresh(global, file);
    } else {
        d->refreshTimer.reset(new QTimer);
        d->refreshTimer->setSingleShot(true);
        connect(d->refreshTimer.get(), &QTimer::timeout, this, [this, global, file]() {
            d->doRefresh(global, file);
        });

        d->refreshTimer->start(ms);
    }
}

void CollectionModel::update()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()) - 1, 0));
}

bool CollectionModel::fetch(const QList<QUrl> &urls)
{
    int row = d->fileList.count();
    beginInsertRows(rootIndex(), row, row + urls.count() - 1);

    d->fileList.append(urls);
    for (const QUrl &url : urls)
        d->fileMap.insert(url, d->shell->fileInfo(d->shell->index(url)));

    endInsertRows();

    return true;
}

bool CollectionModel::take(const QList<QUrl> &urls)
{
    // remove one by one
    for (const QUrl &url : urls) {
        int row = d->fileList.indexOf(url);
        if (row < 0)
            continue;

        beginRemoveRows(rootIndex(), row, row);
        d->fileList.removeAt(row);
        d->fileMap.remove(url);
        endRemoveRows();
    }

    return true;
}

QModelIndex CollectionModel::mapToSource(const QModelIndex &proxyIndex) const
{
    auto url = fileUrl(proxyIndex);
    if (!url.isValid())
        return QModelIndex();

    return d->shell->index(url);
}

QModelIndex CollectionModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    auto url = d->shell->fileUrl(sourceIndex);
    if (!url.isValid())
        return QModelIndex();

    return index(url);
}

QModelIndex CollectionModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || d->fileList.count() <= row)
        return QModelIndex();

    auto url = d->fileList.at(row);
    if (d->fileMap.contains(url))
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex CollectionModel::parent(const QModelIndex &child) const
{
    if (child != rootIndex() && child.isValid())
        return rootIndex();

    return QModelIndex();
}

int CollectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return d->fileList.count();

    return 0;
}

int CollectionModel::columnCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return 1;

    return 0;
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
    QModelIndex sourceIndex = mapToSource(index);
    if (!index.isValid() || !sourceIndex.isValid())
        return QVariant();

    return sourceModel()->data(sourceIndex, role);
}

QMimeData *CollectionModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mm = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << fileUrl(idx);
    mm->setText(kDdeDesktopOrganizer);
    mm->setUrls(urls);
    mm->setData(DFMGLOBAL_NAMESPACE::Mime::kDFMAppTypeKey, QByteArray(kDdeDesktopOrganizer));
    // set user id
    SysInfoUtils::setMimeDataUserId(mm);

    return mm;
}

bool CollectionModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QList<QUrl> urlList = data->urls();
    if (urlList.isEmpty()) {
        fmDebug() << "No URLs in drop data";
        return false;
    }

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = fileUrl(rootIndex());
        fmInfo() << "drop file to desktop" << targetFileUrl << "data" << urlList << action;
    } else {
        targetFileUrl = fileUrl(parent);
        fmInfo() << "drop file to " << targetFileUrl << "data:" << urlList << action;
    }

    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(targetFileUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        fmInfo() << "create FileInfo error: " << errString << targetFileUrl;
        return false;
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

    if (itemInfo->isAttributes(OptInfoType::kIsSymLink)) {
        targetFileUrl = QUrl::fromLocalFile(itemInfo->pathOf(PathInfoType::kSymLinkTarget));
    }

    if (DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        FileOperatorIns->dropToTrash(treeSelectUrl.isEmpty() ? urlList : treeSelectUrl);
        fmInfo() << "Files dropped to trash";
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        // nothing to do.
        fmDebug() << "Drop to computer desktop file, no action needed";
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFileSuffix(targetFileUrl)) {
        FileOperatorIns->dropToApp(urlList, targetFileUrl.toLocalFile());
        fmInfo() << "Files dropped to application:" << targetFileUrl.toString();
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction:
        if (urlList.count() > 0)
            FileOperatorIns->dropFilesToCanvas(action, targetFileUrl, treeSelectUrl.isEmpty() ? urlList : treeSelectUrl);
        break;
    case Qt::LinkAction:
        break;
    default:
        return false;
    }

    return true;
}

void CollectionModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_UNUSED(sourceModel);
    fmWarning() << "forbid setting source model";
    return;
}
