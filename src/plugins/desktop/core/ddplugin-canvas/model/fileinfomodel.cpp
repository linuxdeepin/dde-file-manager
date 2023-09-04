// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfomodel_p.h"
#include "fileprovider.h"
#include "filefilter.h"
#include "utils/fileutil.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>

#include <dfm-framework/dpf.h>

#include <QMimeData>
#include <QDateTime>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileInfoModelPrivate::FileInfoModelPrivate(FileInfoModel *qq)
    : QObject(qq), q(qq)
{
}

void FileInfoModelPrivate::doRefresh()
{
    modelState = FileInfoModelPrivate::RefreshState;
    fileProvider->refresh(filters);
}

QIcon FileInfoModelPrivate::fileIcon(FileInfoPointer info)
{
    using namespace dfmbase::Global;
    const auto &vaule = info->extendAttributes(ExtInfoType::kFileThumbnail);
    if (!vaule.isValid()) {
        ThumbnailFactory::instance()->joinThumbnailJob(info->urlOf(UrlInfoType::kUrl), Global::kLarge);
        // make sure the thumbnail is generated only once
        info->setExtendedAttributes(ExtInfoType::kFileThumbnail, QIcon());
    } else {
        const auto &thumbIcon = vaule.value<QIcon>();
        if (!thumbIcon.isNull())
            return thumbIcon;
    }

    return info->fileIcon();
}

void FileInfoModelPrivate::resetData(const QList<QUrl> &urls)
{
    qDebug() << "to reset file, count:" << urls.size();
    QList<QUrl> fileUrls;
    QMap<QUrl, FileInfoPointer> fileMaps;
    for (const QUrl &child : urls) {
        if (auto itemInfo = FileCreator->createFileInfo(child)) {
            fileUrls.append(itemInfo->urlOf(UrlInfoType::kUrl));
            fileMaps.insert(itemInfo->urlOf(UrlInfoType::kUrl), itemInfo);
        }
    }

    q->beginResetModel();
    {
        QWriteLocker lk(&lock);
        fileList = fileUrls;
        fileMap = fileMaps;
    }

    modelState = FileInfoModelPrivate::NormalState;
    q->endResetModel();
}

void FileInfoModelPrivate::insertData(const QUrl &url)
{
    int row = -1;
    {
        QReadLocker lk(&lock);
        if (auto cur = fileMap.value(url)) {
            lk.unlock();
            qInfo() << "the file to insert is existed" << url;
            cur->refresh(); // refresh fileinfo.
            const QModelIndex &index = q->index(url);
            emit q->dataChanged(index, index);
            return;
        }
        row = fileList.count();
    }

    auto itemInfo = FileCreator->createFileInfo(url);
    if (Q_UNLIKELY(!itemInfo)) {
        qWarning() << "fail to create file info" << url;
        return;
    }

    q->beginInsertRows(q->rootIndex(), row, row);
    {
        QWriteLocker lk(&lock);
        fileList.append(url);
        fileMap.insert(url, itemInfo);
    }
    q->endInsertRows();
}

void FileInfoModelPrivate::removeData(const QUrl &url)
{
    int position = -1;
    {
        QReadLocker lk(&lock);
        position = fileList.indexOf(url);
    }

    if (Q_UNLIKELY(position < 0)) {
        qInfo() << "file dose not exists:" << url;
        return;
    }

    q->beginRemoveRows(q->rootIndex(), position, position);
    {
        QWriteLocker lk(&lock);
        position = fileList.indexOf(url);
        fileList.removeAt(position);
        fileMap.remove(url);
    }
    q->endRemoveRows();
}

void FileInfoModelPrivate::replaceData(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (newUrl.isEmpty()) {
        qInfo() << "target url is empty, remove old" << oldUrl;
        removeData(oldUrl);
        return;
    }

    auto newInfo = FileCreator->createFileInfo(newUrl);
    if (Q_UNLIKELY(newInfo.isNull())) {
        qWarning() << "fail to create new file info:" << newUrl << "old" << oldUrl;
        removeData(oldUrl);
        return;
    }

    {
        QWriteLocker lk(&lock);
        int position = fileList.indexOf(oldUrl);
        if (Q_LIKELY(position < 0)) {
            if (!fileMap.contains(newUrl)) {
                lk.unlock();
                insertData(newUrl);
                return;
            }
        } else {
            if (fileList.contains(newUrl)) {
                // e.g. a mv to b(b is existed)
                //! emit replace signal first.
                emit q->dataReplaced(oldUrl, newUrl);

                // then remove and emit remove signal.
                lk.unlock();
                removeData(oldUrl);
                lk.relock();
                position = fileList.indexOf(newUrl);
                auto cur = fileMap.value(newUrl);
                lk.unlock();

                // refresh file
                cur->refresh();
                qInfo() << "move file" << oldUrl << "to overwritte" << newUrl;
            } else {
                fileList.replace(position, newUrl);
                fileMap.remove(oldUrl);
                fileMap.insert(newUrl, newInfo);
                lk.unlock();
                emit q->dataReplaced(oldUrl, newUrl);
            }

            auto index = q->index(position);
            emit q->dataChanged(index, index);
        }
    }
}

void FileInfoModelPrivate::updateData(const QUrl &url)
{
    {
        QReadLocker lk(&lock);
        if (Q_UNLIKELY(!fileMap.contains(url)))
            return;

        // Although the files cached in InfoCache will be refreshed automatically,
        // a redundant refresh is still required here, because the current variant of FileInfo
        // (like DesktopFileInfo created from DesktopFileCreator) is not in InfoCache and will not be refreshed automatically.
        if (auto info = fileMap.value(url))
            info->refresh();
    }

    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid()))
        return;

    emit q->dataChanged(index, index, {Global::kItemCreateFileInfoRole});
}

void FileInfoModelPrivate::dataUpdated(const QUrl &url, const bool isLinkOrg)
{
    {
        QReadLocker lk(&lock);
        if (Q_UNLIKELY(!fileMap.contains(url)))
            return;
    }

    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid()))
        return;

    auto info = q->fileInfo(index);
    if (info)
        info->customData(Global::ItemRoles::kItemFileRefreshIcon);

    emit q->dataChanged(index, index);
}

void FileInfoModelPrivate::thumbUpdated(const QUrl &url, const QString &thumb)
{
    using namespace dfmbase::Global;
    FileInfoPointer info { nullptr };
    {
        QReadLocker lk(&lock);
        if (Q_UNLIKELY(!fileMap.contains(url)))
            return;

        if (!(info = fileMap.value(url)))
            return;
    }
    // Creating thumbnail icon in a thread may cause the program to crash
    QIcon thumbIcon(thumb);
    if (thumbIcon.isNull())
        return;

    info->setExtendedAttributes(ExtInfoType::kFileThumbnail, thumbIcon);
    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid()))
        return;

    emit q->dataChanged(index, index, {kItemIconRole});
}

FileInfoModel::FileInfoModel(QObject *parent)
    : QAbstractItemModel(parent),
      d(new FileInfoModelPrivate(this))
{
    d->fileProvider = new FileProvider(this);
    installFilter(QSharedPointer<FileFilter>(new RedundantUpdateFilter(d->fileProvider)));

    connect(d->fileProvider, &FileProvider::refreshEnd, d, &FileInfoModelPrivate::resetData);

    connect(d->fileProvider, &FileProvider::fileInserted, d, &FileInfoModelPrivate::insertData);
    connect(d->fileProvider, &FileProvider::fileRemoved, d, &FileInfoModelPrivate::removeData);
    connect(d->fileProvider, &FileProvider::fileUpdated, d, &FileInfoModelPrivate::updateData);
    connect(d->fileProvider, &FileProvider::fileRenamed, d, &FileInfoModelPrivate::replaceData);
    connect(d->fileProvider, &FileProvider::fileInfoUpdated, d, &FileInfoModelPrivate::dataUpdated);
    connect(d->fileProvider, &FileProvider::fileThumbUpdated, d, &FileInfoModelPrivate::thumbUpdated);
}

FileInfoModel::~FileInfoModel()
{
}

QModelIndex FileInfoModel::setRootUrl(QUrl url)
{
    if (url.isEmpty())
        url = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath));

    d->fileProvider->setRoot(url);

    //! FileInfoModel should get all files
    d->filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;

    // root url changed,refresh data as soon
    d->doRefresh();

    return rootIndex();
}

QUrl FileInfoModel::rootUrl() const
{
    return d->fileProvider->root();
}

QModelIndex FileInfoModel::rootIndex() const
{
    return createIndex(INT_MAX, 0, static_cast<void *>(const_cast<FileInfoModel *>(this)));
}

void FileInfoModel::installFilter(QSharedPointer<FileFilter> filter)
{
    d->fileProvider->installFileFilter(filter);
}

void FileInfoModel::removeFilter(QSharedPointer<FileFilter> filter)
{
    d->fileProvider->removeFileFilter(filter);
}

QModelIndex FileInfoModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || rowCount(rootIndex()) <= row)
        return QModelIndex();

    auto url = d->fileList.at(row);
    if (d->fileMap.contains(url))
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex FileInfoModel::index(const QUrl &url, int column) const
{
    if (url.isEmpty())
        return QModelIndex();

    if (d->fileMap.contains(url)) {
        int row = d->fileList.indexOf(url);
        return createIndex(row, column);
    }

    if (url == rootUrl())
        return rootIndex();

    return QModelIndex();
}

FileInfoPointer FileInfoModel::fileInfo(const QModelIndex &index) const
{
    if (index == rootIndex())
        return FileCreator->createFileInfo(rootUrl());

    if (index.row() < 0 || index.row() >= d->fileList.count())
        return nullptr;

    return d->fileMap.value(d->fileList.at(index.row()));
}

QUrl FileInfoModel::fileUrl(const QModelIndex &index) const
{
    if (index == rootIndex())
        return rootUrl();

    if ((index.row() < 0) || (index.row() >= d->fileList.count()))
        return QUrl();

    return d->fileList.at(index.row());
}

QList<QUrl> FileInfoModel::files() const
{
    return d->fileList;
}

void FileInfoModel::refresh(const QModelIndex &parent)
{
    if (parent != rootIndex())
        return;

    d->doRefresh();
}

int FileInfoModel::modelState() const
{
    return d->modelState;
}

void FileInfoModel::update()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()) - 1, 0));
}

void FileInfoModel::updateFile(const QUrl &url)
{
    d->updateData(url);
}

QModelIndex FileInfoModel::parent(const QModelIndex &child) const
{
    if (child != rootIndex() && child.isValid())
        return rootIndex();

    return QModelIndex();
}

int FileInfoModel::rowCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return d->fileList.count();

    return 0;
}

int FileInfoModel::columnCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return 1;

    return 0;
}

QVariant FileInfoModel::data(const QModelIndex &index, int itemRole) const
{
    if (!index.isValid() || index.model() != this || index == rootIndex())
        return QVariant();

    auto indexFileInfo = fileInfo(index);
    if (!indexFileInfo) {
        return QVariant();
    }
    switch (itemRole) {
    case Global::ItemRoles::kItemIconRole:
        return d->fileIcon(indexFileInfo);
    case Global::ItemRoles::kItemNameRole:
        return indexFileInfo->nameOf(NameInfoType::kFileName);
    case Qt::EditRole:
    case Global::ItemRoles::kItemFileDisplayNameRole:
        return indexFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    case Global::ItemRoles::kItemFilePinyinNameRole:
        return indexFileInfo->displayOf(DisPlayInfoType::kFileDisplayPinyinName);
    case Global::ItemRoles::kItemFileLastModifiedRole:
        return indexFileInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>().toString("yyyy/MM/dd HH:mm:ss");   // todo by file info: lastModifiedDisplayName
    case Global::ItemRoles::kItemFileSizeRole:
        return indexFileInfo->isAttributes(OptInfoType::kIsDir) ? indexFileInfo->countChildFile() : indexFileInfo->size();
    case Global::ItemRoles::kItemFileMimeTypeRole:
        return indexFileInfo->fileMimeType().name();   // todo by file info: mimeTypeDisplayName
    case Global::ItemRoles::kItemExtraProperties:
        return indexFileInfo->extraProperties();
    case Global::ItemRoles::kItemFileBaseNameRole:
        return indexFileInfo->nameOf(NameInfoType::kBaseName);
    case Global::ItemRoles::kItemFileSuffixRole:
        return indexFileInfo->nameOf(NameInfoType::kSuffix);
    case Global::ItemRoles::kItemFileNameOfRenameRole:
        return indexFileInfo->nameOf(NameInfoType::kFileNameOfRename);
    case Global::ItemRoles::kItemFileBaseNameOfRenameRole:
        return indexFileInfo->nameOf(NameInfoType::kBaseNameOfRename);
    case Global::ItemRoles::kItemFileSuffixOfRenameRole:
        return indexFileInfo->nameOf(NameInfoType::kSuffixOfRename);
    default:
        return QString();
    }
}

Qt::ItemFlags FileInfoModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    flags |= Qt::ItemIsDragEnabled;

    if (auto file = fileInfo(index)) {
        if (file->canAttributes(CanableInfoType::kCanRename))
            flags |= Qt::ItemIsEditable;

        if (file->isAttributes(OptInfoType::kIsWritable))
            flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

QMimeData *FileInfoModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mmData = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << fileUrl(idx);

    mmData->setUrls(urls);
    return mmData;
}

bool FileInfoModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QList<QUrl> urlList = data->urls();
    if (urlList.isEmpty())
        return false;

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = rootUrl();
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

    if (DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0, urlList, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        // nothing to do.
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFile(targetFileUrl)) {
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, urlList, QStringList { targetFileUrl.toLocalFile() });
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction: {
        if (action == Qt::MoveAction) {
            if (urlList.count() > 0)
                dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, urlList, targetFileUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        } else {
            // default is copy file
            if (urlList.count() > 0)
                dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, urlList, targetFileUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        }
    } break;
    case Qt::LinkAction:
        break;
    default:
        return false;
    }

    return true;
}

QStringList FileInfoModel::mimeTypes() const
{
    static QStringList types { QLatin1String("text/uri-list") };
    return types;
}

Qt::DropActions FileInfoModel::supportedDragActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FileInfoModel::supportedDropActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}
