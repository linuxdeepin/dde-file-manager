// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QApplication>
#include <QTimer>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileInfoModelPrivate::FileInfoModelPrivate(FileInfoModel *qq)
    : QObject(qq), q(qq)
{
}

void FileInfoModelPrivate::doRefresh()
{
    FileUtils::refreshIconCache();
    modelState = FileInfoModelPrivate::RefreshState;
    fileProvider->refresh(filters);
}

QIcon FileInfoModelPrivate::fileIcon(FileInfoPointer info)
{
    using namespace dfmbase::Global;
    const auto &value = info->extendAttributes(ExtInfoType::kFileThumbnail);
    if (!value.isValid()) {
        ThumbnailFactory::instance()->joinThumbnailJob(info->urlOf(UrlInfoType::kUrl), Global::kLarge);
        // make sure the thumbnail is generated only once
        info->setExtendedAttributes(ExtInfoType::kFileThumbnail, QIcon());
    } else {
        const auto &thumbIcon = value.value<QIcon>();
        if (!thumbIcon.isNull())
            return thumbIcon;
    }

    return info->fileIcon();
}

void FileInfoModelPrivate::resetData(const QList<QUrl> &urls)
{
    fmDebug() << "Resetting file info model data with" << urls.size() << "files";
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
            fmInfo() << "File already exists in model, refreshing:" << url;
            cur->refresh();   // refresh fileinfo.
            const QModelIndex &index = q->index(url);
            emit q->dataChanged(index, index);
            return;
        }
        row = fileList.count();
    }

    auto itemInfo = FileCreator->createFileInfo(url);
    if (Q_UNLIKELY(!itemInfo)) {
        fmWarning() << "Failed to create file info for insertion:" << url;
        return;
    }

    itemInfo->updateAttributes();
    q->beginInsertRows(q->rootIndex(), row, row);
    {
        QWriteLocker lk(&lock);
        fileList.append(url);
        fileMap.insert(url, itemInfo);
    }
    q->endInsertRows();

    // Since QIcon::fromTheme stores the QIcon as NULL for the cache,
    // this results in the cache not being updated when the icon is drawn for
    // the first time if the cached icon is empty, even if the icon icon resource is installed next.
    //
    // detail see: https://bugreports.qt.io/browse/QTBUG-112257
    if (FileUtils::isDesktopFileSuffix(itemInfo->fileUrl())) {
        checkAndRefreshDesktopIcon(itemInfo);
    }
}

void FileInfoModelPrivate::removeData(const QUrl &url)
{
    int position = -1;
    {
        QReadLocker lk(&lock);
        position = fileList.indexOf(url);
    }

    if (Q_UNLIKELY(position < 0)) {
        fmDebug() << "File not found in model for removal:" << url;
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
        fmInfo() << "Target URL is empty, removing old file:" << oldUrl;
        removeData(oldUrl);
        return;
    }

    // check the newUrl whether has been in cache.
    auto cachedInfo = InfoCacheController::instance().getCacheInfo(newUrl);
    auto newInfo = FileCreator->createFileInfo(newUrl);
    if (Q_UNLIKELY(newInfo.isNull())) {
        fmWarning() << "Failed to create new file info for replacement - old:" << oldUrl << "new:" << newUrl;
        removeData(oldUrl);
        return;
    }

    {
        QWriteLocker lk(&lock);
        int position = fileList.indexOf(oldUrl);
        if (Q_LIKELY(position < 0)) {
            if (!fileMap.contains(newUrl)) {
                lk.unlock();
                fmDebug() << "Old URL not in model, inserting new URL:" << newUrl;
                insertData(newUrl);
                return;
            }
        } else {
            if (fileList.contains(newUrl)) {
                // e.g. a mv to b(b is existed)
                //! emit replace signal first.
                fmInfo() << "Target URL already exists, handling overwrite - old:" << oldUrl << "new:" << newUrl;
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
                fmInfo() << "File moved to overwrite existing file:" << oldUrl << "->" << newUrl;
            } else {
                fileList.replace(position, newUrl);
                fileMap.remove(oldUrl);
                fileMap.insert(newUrl, newInfo);
                lk.unlock();

                // refresh file because an old info cahe may exist.
                if (cachedInfo == newInfo)
                    newInfo->refresh();

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
        if (Q_UNLIKELY(!fileMap.contains(url))) {
            fmDebug() << "File not in model for update:" << url;
            return;
        }

        // Although the files cached in InfoCache will be refreshed automatically,
        // a redundant refresh is still required here, because the current variant of FileInfo
        // (like DesktopFileInfo created from DesktopFileCreator) is not in InfoCache and will not be refreshed automatically.
        if (auto info = fileMap.value(url))
            info->updateAttributes();
    }

    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid())) {
        fmWarning() << "Invalid model index for file update:" << url;
        return;
    }

    emit q->dataChanged(index, index, { Global::kItemCreateFileInfoRole });
}

void FileInfoModelPrivate::dataUpdated(const QUrl &url, const bool isLinkOrg)
{
    {
        QReadLocker lk(&lock);
        if (Q_UNLIKELY(!fileMap.contains(url))) {
            fmDebug() << "File not in model for data update:" << url;
            return;
        }
    }

    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid())) {
        fmWarning() << "Invalid model index for data update:" << url;
        return;
    }

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
        if (Q_UNLIKELY(!fileMap.contains(url))) {
            fmDebug() << "File not in model for thumbnail update:" << url;
            return;
        }

        if (!(info = fileMap.value(url))) {
            fmWarning() << "File info not found for thumbnail update:" << url;
            return;
        }
    }

    // Creating thumbnail icon in a thread may cause the program to crash
    QIcon thumbIcon(thumb);
    if (thumbIcon.isNull()) {
        fmWarning() << "Failed to create thumbnail icon from path:" << thumb;
        return;
    }

    info->setExtendedAttributes(ExtInfoType::kFileThumbnail, thumbIcon);
    const QModelIndex &index = q->index(url);
    if (Q_UNLIKELY(!index.isValid())) {
        fmWarning() << "Invalid model index for thumbnail update:" << url;
        return;
    }

    emit q->dataChanged(index, index, { kItemIconRole });
}

void FileInfoModelPrivate::checkAndRefreshDesktopIcon(const FileInfoPointer &info, int retryCount)
{
    if (retryCount < 0) {
        fmWarning() << "Desktop icon refresh retries exhausted, trying XDG fallback for:" << info->urlOf(UrlInfoType::kUrl);
        // All retries exhausted, try qtxdg-iconfinder as last resort
        DesktopFile file(info->absoluteFilePath());
        QString iconName = file.desktopIcon();

        // NOTE: FileUtils::findIconFromXdg is very slow!
        // Maybe cause UI blocking
        QString iconPath = FileUtils::findIconFromXdg(iconName);
        fmWarning() << "XDG icon search result for" << iconName << ":" << iconPath;
        if (!iconPath.isEmpty()) {
            FileUtils::refreshIconCache();
            updateData(info->urlOf(UrlInfoType::kUrl));
        }
        return;
    }

    DesktopFile file(info->absoluteFilePath());
    QString iconName = file.desktopIcon();
    bool isNullIcon = QIcon::fromTheme(iconName).isNull();
    if (!isNullIcon) {
        fmDebug() << "Desktop icon found for file:" << info->urlOf(UrlInfoType::kUrl) << "icon:" << iconName;
        return;
    }

    // When installing a deb package, the desktop file may be installed before its icon resources.
    // We need to retry checking the icon multiple times to ensure it's properly loaded after
    // the icon resources are installed. Each retry has a 2-second interval, with a maximum of
    // 5 retries (10 seconds total). If all retries fail, we'll try qtxdg-iconfinder as last resort.
    QTimer::singleShot(2000, this, [this, info, retryCount]() {
        FileUtils::refreshIconCache();
        updateData(info->urlOf(UrlInfoType::kUrl));

        // Recursive retry with decremented counter
        checkAndRefreshDesktopIcon(info, retryCount - 1);
    });
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
    if (url.isEmpty()) {
        url = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath));
        fmDebug() << "Empty root URL provided, using default desktop path:" << url;
    }

    fmInfo() << "Setting file info model root URL to:" << url;
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
    if (parent != rootIndex()) {
        fmDebug() << "Refresh requested for non-root index, ignoring";
        return;
    }

    fmInfo() << "Refreshing file info model";
    d->doRefresh();
}

int FileInfoModel::modelState() const
{
    return d->modelState;
}

void FileInfoModel::update()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->updateAttributes();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()) - 1, 0));
}

void FileInfoModel::updateFile(const QUrl &url)
{
    d->updateData(url);
}

void FileInfoModel::refreshAllFile()
{
    for (auto itor = d->fileMap.begin(); itor != d->fileMap.end(); ++itor)
        itor.value()->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()) - 1, 0));
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
        fmWarning() << "File info not found for index row:" << index.row();
        return QVariant();
    }
    switch (itemRole) {
    case Global::ItemRoles::kItemFontRole:
        return qApp->font();
    case Global::ItemRoles::kItemIconRole:
        return d->fileIcon(indexFileInfo);
    case Global::ItemRoles::kItemNameRole:
        return indexFileInfo->nameOf(NameInfoType::kFileName);
    case Qt::EditRole:
    case Global::ItemRoles::kItemFileDisplayNameRole:
        return indexFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    case Global::ItemRoles::kItemFilePinyinNameRole:
        return indexFileInfo->displayOf(DisPlayInfoType::kFileDisplayPinyinName);
    case Global::ItemRoles::kItemFileCreatedRole:
        return indexFileInfo->timeOf(TimeInfoType::kCreateTime).value<QDateTime>().toString("yyyy/MM/dd HH:mm:ss");
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
    case Global::ItemRoles::kItemFileIconRole:
            return indexFileInfo->fileIcon();
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

        // use can drop attribute,if error modify the fileinfo candrop attribute
        if (file->canAttributes(CanableInfoType::kCanDrop))
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
    if (urlList.isEmpty()) {
        fmWarning() << "Empty URL list in drop mime data";
        return false;
    }

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = rootUrl();
        fmInfo() << "Dropping" << urlList.size() << "files to desktop:" << targetFileUrl << "action:" << action;
    } else {
        targetFileUrl = fileUrl(parent);
        fmInfo() << "Dropping" << urlList.size() << "files to:" << targetFileUrl << "action:" << action;
    }

    auto itemInfo = FileCreator->createFileInfo(targetFileUrl);
    if (Q_UNLIKELY(!itemInfo)) {
        fmWarning() << "Failed to create file info for drop target:" << targetFileUrl;
        return false;
    }

    if (itemInfo->isAttributes(OptInfoType::kIsSymLink)) {
        targetFileUrl = QUrl::fromLocalFile(itemInfo->pathOf(PathInfoType::kSymLinkTarget));
        fmDebug() << "Drop target is symlink, resolving to:" << targetFileUrl;
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
        fmInfo() << "Dropping files to trash";
        dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0,
                                     treeSelectUrl.isEmpty() ? urlList : treeSelectUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        fmDebug() << "Drop to computer desktop file ignored";
        // nothing to do.
        return true;
    } else if (DFMBASE_NAMESPACE::FileUtils::isDesktopFileSuffix(targetFileUrl)) {
        fmInfo() << "Dropping files to desktop application:" << targetFileUrl.toLocalFile();
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, urlList, QStringList { targetFileUrl.toLocalFile() });
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction: {
        if (action == Qt::MoveAction) {
            if (urlList.count() > 0)
                dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, treeSelectUrl.isEmpty() ? urlList : treeSelectUrl,
                                             targetFileUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        } else {
            // default is copy file
            if (urlList.count() > 0)
                dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, treeSelectUrl.isEmpty() ? urlList : treeSelectUrl,
                                             targetFileUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr);
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
