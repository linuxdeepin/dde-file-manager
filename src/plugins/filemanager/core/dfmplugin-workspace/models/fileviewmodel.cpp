// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewmodel.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperatorhelper.h"
#include "utils/filedatamanager.h"
#include "utils/filesortworker.h"
#include "models/rootinfo.h"
#include "models/fileitemdata.h"
#include "events/workspaceeventsequence.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QPointer>
#include <QList>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel(parent)

{
    currentKey = QString::number(quintptr(this), 16);
    itemRootData = new FileItemData(dirRootUrl);
    connect(ThumbnailFactory::instance(), &ThumbnailFactory::produceFinished, this, &FileViewModel::onFileThumbUpdated);
    connect(Application::instance(), &Application::genericAttributeChanged, this, &FileViewModel::onGenericAttributeChanged);
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &FileViewModel::onDConfigChanged);
    connect(&waitTimer, &QTimer::timeout, this, &FileViewModel::onSetCursorWait);
    waitTimer.setInterval(50);
}

FileViewModel::~FileViewModel()
{
    closeCursorTimer();
    quitFilterSortWork();

    if (itemRootData) {
        delete itemRootData;
        itemRootData = nullptr;
    }
    FileDataManager::instance()->cleanRoot(dirRootUrl, currentKey);
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    auto isParentValid = parent.isValid();

    if ((!isParentValid && (row != 0 || column != 0))
        || (row < 0 || column < 0))
        return QModelIndex();

    if (!isParentValid && filterSortWorker.isNull())
        return createIndex(row, column, itemRootData);

    if (!filterSortWorker)
        return QModelIndex();

    FileItemData *itemData = nullptr;
    if (!isParentValid) {
        itemData = filterSortWorker->rootData();
    } else {
        itemData = filterSortWorker->childData(row);
    }

    return createIndex(row, column, itemData);
}

QUrl FileViewModel::rootUrl() const
{
    return dirRootUrl;
}

QModelIndex FileViewModel::rootIndex() const
{
    if (!filterSortWorker)
        return QModelIndex();

    auto data = filterSortWorker->rootData();
    if (data) {
        return createIndex(0, 0, data);
    } else {
        return QModelIndex();
    }
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    if (!url.isValid())
        return QModelIndex();

    // insert root index
    beginResetModel();
    closeCursorTimer();
    // create root by url
    dirRootUrl = url;
    RootInfo *root = FileDataManager::instance()->fetchRoot(dirRootUrl);
    endResetModel();

    initFilterSortWork();

    // connect signals
    connect(root, &RootInfo::requestCloseTab, this, [](const QUrl &url) { WorkspaceHelper::instance()->closeTab(url); }, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::getSourceData, root, &RootInfo::handleGetSourceData, Qt::QueuedConnection);
    connect(root, &RootInfo::sourceDatas, filterSortWorker.data(), &FileSortWorker::handleSourceChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::iteratorLocalFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorLocalChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::iteratorAddFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherAddFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherAddChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherRemoveFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherRemoveChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherUpdateFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateFile, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherUpdateHideFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateHideFile, Qt::QueuedConnection);
    connect(root, &RootInfo::traversalFinished, filterSortWorker.data(), &FileSortWorker::handleTraversalFinish, Qt::QueuedConnection);
    connect(root, &RootInfo::requestSort, filterSortWorker.data(), &FileSortWorker::handleSortAll, Qt::QueuedConnection);

    // fetch files
    const QModelIndex &index = rootIndex();

    if (WorkspaceHelper::instance()->haveViewRoutePrehandler(url.scheme())) {
        auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
        if (prehandler) {
            quint64 winId = FileManagerWindowsManager::instance().findWindowId(qobject_cast<FileView *>(QObject::parent()));
            prehandler(winId, url, [this, index]() {
                this->canFetchFiles = true;
                this->fetchMore(index);
            });
        }
    } else {
        canFetchFiles = true;
        fetchMore(index);
    }

    return index;
}

FileInfoPointer FileViewModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || filterSortWorker.isNull())
        return nullptr;

    const QModelIndex &parentIndex = index.parent();
    FileItemData *item = nullptr;
    if (!parentIndex.isValid()) {
        item = filterSortWorker->rootData();
    } else {
        item = filterSortWorker->childData(index.row());
    }

    if (!item)
        return nullptr;

    if (item == filterSortWorker->rootData()) {
        if (!item->fileInfo())
            item->data(Global::ItemRoles::kItemCreateFileInfoRole);

        if (!item->fileInfo())
            return InfoFactory::create<FileInfo>(item->data(Global::ItemRoles::kItemUrlRole).value<QUrl>());
    }

    return item->fileInfo();
}

QList<QUrl> FileViewModel::getChildrenUrls() const
{
    if (filterSortWorker)
        return filterSortWorker->getChildrenUrls();

    return {};
}

QModelIndex FileViewModel::getIndexByUrl(const QUrl &url) const
{
    if (!filterSortWorker)
        return QModelIndex();

    int rowIndex = filterSortWorker->getChildShowIndex(url);

    if (rowIndex >= 0)
        return index(rowIndex, 0, rootIndex());

    return QModelIndex();
}

int FileViewModel::getColumnWidth(int column) const
{
    const ItemRoles role = getRoleByColumn(column);

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();
    int colWidth = state.value(QString::number(role), -1).toInt();
    if (colWidth > 0) {
        return colWidth;
    }

    // default width of each column
    return 120;
}

ItemRoles FileViewModel::getRoleByColumn(int column) const
{
    QList<ItemRoles> columnRoleList = getColumnRoles();

    if (columnRoleList.length() > column)
        return columnRoleList.at(column);

    return kItemFileDisplayNameRole;
}

int FileViewModel::getColumnByRole(ItemRoles role) const
{
    QList<ItemRoles> columnRoleList = getColumnRoles();
    return columnRoleList.indexOf(role) < 0 ? 0 : columnRoleList.indexOf(role);
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    const FileItemData *childData = static_cast<FileItemData *>(child.internalPointer());

    if (childData && childData->parentData())
        return index(0, 0, QModelIndex());

    return QModelIndex();
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    /*  notes:
        It's no need to return 1 when parent is invalid, because there is no such scenario.
        AT tool(sniff) will use this return value to make AT-tag and show each item.
        If the return value is greater than 0, the AT-tags are shown normally and item
        count is equal to the return value;
        For AT-tool, if `parent` is invalid and return 1, then only 1 item would
        be shown in AT-tool, because at that time `filterSortWorker` has not get correct children count,
        but the AT-tool need a correct children count, not 1.
    */
    if (!filterSortWorker.isNull()) {
        return filterSortWorker->childrenCount();
    }

    return 0;
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return getColumnRoles().count();
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    const QModelIndex &parentIndex = index.parent();

    if (filterSortWorker.isNull())
        return QVariant();

    FileItemData *itemData = nullptr;
    int columnRole = role;
    if (!parentIndex.isValid()) {
        itemData = filterSortWorker->rootData();
    } else {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole: {
            ItemRoles temRole = columnToRole(index.column());
            if (temRole != ItemRoles::kItemUnknowRole)
                columnRole = temRole;
        } break;
        default:
            break;
        }
        itemData = filterSortWorker->childData(index.row());
    }

    if (itemData) {
        return itemData->data(columnRole);
    } else {
        return QVariant();
    }
}

QVariant FileViewModel::headerData(int column, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        int column_role = getRoleByColumn(column);
        return roleDisplayString(column_role);
    }

    return QVariant();
}

void FileViewModel::refresh()
{
    FileDataManager::instance()->cleanRoot(dirRootUrl, currentKey, true);

    Q_EMIT requestRefreshAllChildren();
}

ModelState FileViewModel::currentState() const
{
    return state;
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    // first fetch: do traversal whole dir
    // other fetch: fetch file list from data cache
    if (!canFetchMore(parent)) {
        QApplication::restoreOverrideCursor();
        return;
    }
    canFetchFiles = false;

    bool ret { false };
    if (filterSortWorker.isNull()) {
        ret = FileDataManager::instance()->fetchFiles(dirRootUrl, currentKey);
    } else {
        ret = FileDataManager::instance()->fetchFiles(dirRootUrl,
                                                      currentKey,
                                                      filterSortWorker->getSortRole(),
                                                      filterSortWorker->getSortOrder());
    }

    if (ret) {
        changeState(ModelState::kBusy);
        startCursorTimer();
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return canFetchFiles;
}

Qt::ItemFlags FileViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (!index.data(kItemFileIsAvailableRole).toBool()) {
        flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        return flags;
    }

    if (index.data(kItemFileCanRenameRole).toBool())
        flags |= Qt::ItemIsEditable;

    if (index.data(kItemFileIsWritableRole).toBool())
        flags |= Qt::ItemIsDropEnabled;

    if (index.data(kItemFileCanDragRole).toBool())
        flags |= Qt::ItemIsDragEnabled;

    if (readOnly)
        flags &= ~(Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);

    return flags;
}

QStringList FileViewModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

QMimeData *FileViewModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QSet<QUrl> urlsSet;
    QList<QModelIndex>::const_iterator it = indexes.begin();

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            FileInfoPointer fileInfo = this->fileInfo(*it);
            if (fileInfo.isNull())
                (*it).data(Global::ItemRoles::kItemCreateFileInfoRole);
            fileInfo = this->fileInfo(*it);
            if (fileInfo.isNull())
                continue;
            const QUrl &url = fileInfo->urlOf(UrlInfoType::kUrl);

            if (urlsSet.contains(url))
                continue;

            urls << url;
            urlsSet << url;
        }
    }

    QMimeData *data = new QMimeData();

    data->setUrls(urls);
    SysInfoUtils::setMimeDataUserId(data);

    return data;
}

bool FileViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    const QModelIndex &dropIndex = index(row, column, parent);

    if (!dropIndex.isValid())
        return false;

    FileInfoPointer targetFileInfo = fileInfo(dropIndex);
    if (targetFileInfo.isNull())
        dropIndex.data(Global::ItemRoles::kItemCreateFileInfoRole);

    targetFileInfo = fileInfo(dropIndex);
    if (!targetFileInfo || (targetFileInfo->isAttributes(OptInfoType::kIsDir) && !targetFileInfo->isAttributes(OptInfoType::kIsWritable))) {
        qWarning() << "Drop target dir is not writable!";
        return false;
    }
    QUrl targetUrl = targetFileInfo->urlOf(UrlInfoType::kUrl);
    const QList<QUrl> &dropUrls = data->urls();

    if (targetFileInfo->isAttributes(OptInfoType::kIsSymLink))
        targetUrl = QUrl::fromLocalFile(targetFileInfo->pathOf(PathInfoType::kSymLinkTarget));

    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());

    if (FileUtils::isTrashDesktopFile(targetUrl)) {
        FileOperatorHelperIns->moveToTrash(view, dropUrls);
        return true;
    } else if (FileUtils::isDesktopFile(targetUrl)) {
        FileOperatorHelperIns->openFilesByApp(view, dropUrls, QStringList { targetUrl.toLocalFile() });
        return true;
    }

    bool ret { true };

    switch (action) {
    case Qt::CopyAction:
        [[fallthrough]];
    case Qt::MoveAction:
        if (dropUrls.count() > 0)
            // call move
            FileOperatorHelperIns->dropFiles(view, action, targetUrl, dropUrls);
        break;
    default:
        break;
    }

    return ret;
}

Qt::DropActions FileViewModel::supportedDragActions() const
{
    auto info = fileInfo(rootIndex());
    if (info)
        return info->supportedOfAttributes(SupportedType::kDrag);

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FileViewModel::supportedDropActions() const
{
    auto info = fileInfo(rootIndex());
    if (info)
        return info->supportedOfAttributes(SupportedType::kDrop);

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

void FileViewModel::sort(int column, Qt::SortOrder order)
{
    ItemRoles role = getRoleByColumn(column);
    Q_EMIT requestSortChildren(order, role,
                               Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool());
}

void FileViewModel::stopTraversWork()
{
    discardFilterSortObjects();
    FileDataManager::instance()->cleanRoot(dirRootUrl, currentKey);

    changeState(ModelState::kIdle);
    closeCursorTimer();
}

QList<ItemRoles> FileViewModel::getColumnRoles() const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(dirRootUrl, &roles);

    const QVariantMap &map = DFMBASE_NAMESPACE::Application::appObtuselySetting()->value("FileViewState", dirRootUrl).toMap();
    if (map.contains("headerList")) {
        QVariantList headerList = map.value("headerList").toList();

        for (ItemRoles role : roles) {
            if (!headerList.contains(role))
                headerList.append(role);
        }

        roles.clear();
        for (auto var : headerList) {
            roles.append(static_cast<ItemRoles>(var.toInt()));
        }
    } else if (!customOnly) {
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                                           << kItemFileLastModifiedRole
                                                                           << kItemFileSizeRole
                                                                           << kItemFileMimeTypeRole;

        int customCount = roles.count();
        for (auto role : defualtColumnRoleList) {
            if (!roles.contains(role))
                roles.insert(roles.length() - customCount, role);
        }
    }

    return roles;
}

ItemRoles FileViewModel::columnToRole(int column) const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(dirRootUrl, &roles);

    const QVariantMap &map = DFMBASE_NAMESPACE::Application::appObtuselySetting()->value("FileViewState", dirRootUrl).toMap();
    if (map.contains("headerList")) {
        QVariantList headerList = map.value("headerList").toList();
        if (headerList.length() > column)
            return ItemRoles(headerList.at(column).toInt());

    } else if (!customOnly) {
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                                           << kItemFileLastModifiedRole
                                                                           << kItemFileSizeRole
                                                                           << kItemFileMimeTypeRole;

        if (defualtColumnRoleList.length() > column) {
            return defualtColumnRoleList.at(column);
        }
    }

    return kItemUnknowRole;
}

QString FileViewModel::roleDisplayString(int role) const
{
    QString displayName;

    if (WorkspaceEventSequence::instance()->doFetchCustomRoleDiaplayName(dirRootUrl, static_cast<ItemRoles>(role), &displayName))
        return displayName;

    switch (role) {
    case kItemFileDisplayNameRole:
        return tr("Name");
    case kItemFileLastModifiedRole:
        return tr("Time modified");
    case kItemFileSizeRole:
        return tr("Size");
    case kItemFileMimeTypeRole:
        return tr("Type");
    default:
        return QString();
    }
}

void FileViewModel::updateFile(const QUrl &url)
{
    Q_EMIT requestUpdateFile(url);
}

Qt::SortOrder FileViewModel::sortOrder() const
{
    if (filterSortWorker.isNull())
        return Qt::AscendingOrder;

    return filterSortWorker->getSortOrder();
}

ItemRoles FileViewModel::sortRole() const
{
    if (filterSortWorker.isNull())
        return kItemFileDisplayNameRole;

    return filterSortWorker->getSortRole();
}

void FileViewModel::setFilters(QDir::Filters filters)
{
    currentFilters = filters;
    Q_EMIT requestChangeFilters(filters);
}

QDir::Filters FileViewModel::getFilters() const
{
    return currentFilters;
}

void FileViewModel::setNameFilters(const QStringList &filters)
{
    if (nameFilters == filters)
        return;

    nameFilters = filters;
    Q_EMIT requestChangeNameFilters(filters);
}

QStringList FileViewModel::getNameFilters() const
{
    return nameFilters;
}

void FileViewModel::setFilterData(const QVariant &data)
{
    filterData = data;
    // 设置要触发重新过滤信号
    Q_EMIT requestSetFilterData(data);
}

void FileViewModel::setFilterCallback(const FileViewFilterCallback callback)
{
    filterCallback = callback;
    // 设置要触发重新过滤信号
    Q_EMIT requestSetFilterCallback(callback);
}

void FileViewModel::toggleHiddenFiles()
{
    Q_EMIT requestChangeHiddenFilter();
}

void FileViewModel::setReadOnly(bool value)
{
    readOnly = value;
}

void FileViewModel::updateThumbnailIcon(const QModelIndex &index, const QString &thumb)
{
    auto info = fileInfo(index);
    if (!info)
        return;

    // Creating thumbnail icon in a thread may cause the program to crash
    QIcon thumbIcon(thumb);
    if (thumbIcon.isNull())
        return;

    info->setExtendedAttributes(ExtInfoType::kFileThumbnail, thumbIcon);
}

void FileViewModel::onFileThumbUpdated(const QUrl &url, const QString &thumb)
{
    auto updateIndex = getIndexByUrl(url);
    if (!updateIndex.isValid())
        return;

    updateThumbnailIcon(updateIndex, thumb);
    auto view = qobject_cast<FileView *>(QObject::parent());
    if (view) {
        view->update(updateIndex);
    } else {
        Q_EMIT dataChanged(updateIndex, updateIndex);
    }
}

void FileViewModel::onFileUpdated(int show)
{
    auto view = qobject_cast<FileView *>(QObject::parent());
    if (view) {
        view->update(index(show, 0, rootIndex()));
    } else {
        Q_EMIT dataChanged(index(show, 0, rootIndex()), index(show, 0, rootIndex()));
    }
}

void FileViewModel::onInsert(int firstIndex, int count)
{
    beginInsertRows(rootIndex(), firstIndex, firstIndex + count - 1);
}

void FileViewModel::onInsertFinish()
{
    endInsertRows();
}

void FileViewModel::onRemove(int firstIndex, int count)
{
    beginRemoveRows(rootIndex(), firstIndex, firstIndex + count - 1);
}

void FileViewModel::onRemoveFinish()
{
    endRemoveRows();
}

void FileViewModel::onUpdateView()
{
    FileView *view = qobject_cast<FileView *>(QObject::parent());
    if (view)
        view->update();
}

void FileViewModel::onGenericAttributeChanged(Application::GenericAttribute ga, const QVariant &value)
{
    Q_UNUSED(value)

    switch (ga) {
    case Application::kPreviewAudio:
    case Application::kPreviewImage:
    case Application::kPreviewVideo:
    case Application::kPreviewTextFile:
    case Application::kPreviewDocumentFile:
    case Application::kShowThunmbnailInRemote:
        Q_EMIT requestClearThumbnail();
        break;
    default:
        break;
    }
}

void FileViewModel::onDConfigChanged(const QString &config, const QString &key)
{
    if (config != DConfigInfo::kConfName)
        return;

    if (DConfigInfo::kMtpThumbnailKey == key && FileUtils::isMtpFile(rootUrl()))
        Q_EMIT requestClearThumbnail();
}

void FileViewModel::onSetCursorWait()
{
    if (currentState() != ModelState::kBusy)
        return;

    if (QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::CursorShape::WaitCursor)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void FileViewModel::initFilterSortWork()
{
    discardFilterSortObjects();
    filterSortThread.reset(new QThread);

    // make filters
    if (currentFilters == QDir::NoFilter) {
        currentFilters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
        bool isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
        if (isShowedHiddenFiles) {
            currentFilters |= QDir::Hidden;
        } else {
            currentFilters &= ~QDir::Hidden;
        }
    }

    // get sort config
    QMap<QString, QVariant> valueMap = Application::appObtuselySetting()->value("FileViewState", dirRootUrl).toMap();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(valueMap.value("sortOrder", Qt::SortOrder::AscendingOrder).toInt());
    ItemRoles role = static_cast<ItemRoles>(valueMap.value("sortRole", kItemFileDisplayNameRole).toInt());

    filterSortWorker.reset(new FileSortWorker(dirRootUrl, currentKey, filterCallback, nameFilters, currentFilters));
    beginInsertRows(QModelIndex(), 0, 0);
    filterSortWorker->setRootData(new FileItemData(dirRootUrl, InfoFactory::create<FileInfo>(dirRootUrl)));
    endInsertRows();
    filterSortWorker->setSortAgruments(order, role, Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool());
    filterSortWorker->moveToThread(filterSortThread.data());

    // connect signals
    connect(filterSortWorker.data(), &FileSortWorker::insertRows, this, &FileViewModel::onInsert, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::insertFinish, this, &FileViewModel::onInsertFinish, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::removeRows, this, &FileViewModel::onRemove, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::removeFinish, this, &FileViewModel::onRemoveFinish, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::requestFetchMore, this, [this]() { canFetchFiles = true; fetchMore(rootIndex()); }, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::updateRow, this, &FileViewModel::onFileUpdated, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::selectAndEditFile, this, &FileViewModel::selectAndEditFile, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::requestSetIdel, this, [this]() {
        this->changeState(ModelState::kIdle);
        closeCursorTimer();
    },
            Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeHiddenFilter, filterSortWorker.data(), &FileSortWorker::onToggleHiddenFiles, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeFilters, filterSortWorker.data(), &FileSortWorker::setFilters, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeNameFilters, filterSortWorker.data(), &FileSortWorker::setNameFilters, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestUpdateFile, filterSortWorker.data(), &FileSortWorker::handleUpdateFile, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestSortChildren, filterSortWorker.data(), &FileSortWorker::resort, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestSetFilterData, filterSortWorker.data(), &FileSortWorker::handleFilterData, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestSetFilterCallback, filterSortWorker.data(), &FileSortWorker::handleFilterCallFunc, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestGetSourceData, filterSortWorker.data(), &FileSortWorker::handleModelGetSourceData, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestRefreshAllChildren, filterSortWorker.data(), &FileSortWorker::handleRefresh, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestClearThumbnail, filterSortWorker.data(), &FileSortWorker::handleClearThumbnail, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::requestUpdateView, this, &FileViewModel::onUpdateView, Qt::QueuedConnection);
    connect(Application::instance(), &Application::showedHiddenFilesChanged, filterSortWorker.data(), &FileSortWorker::onShowHiddenFileChanged, Qt::QueuedConnection);
    connect(Application::instance(), &Application::appAttributeChanged, filterSortWorker.data(), &FileSortWorker::onAppAttributeChanged, Qt::QueuedConnection);

    filterSortThread->start();
}

void FileViewModel::quitFilterSortWork()
{
    if (!filterSortWorker.isNull()) {
        filterSortWorker->disconnect();
        filterSortWorker->cancel();
    }
    if (!filterSortThread.isNull()) {
        filterSortThread->quit();
        filterSortThread->wait();
    }
}

void FileViewModel::discardFilterSortObjects()
{
    if (!filterSortThread.isNull() && !filterSortWorker.isNull()) {
        auto discardedWorker = filterSortWorker;
        discardedWorker->cancel();
        discardedObjects.append(discardedWorker);
        filterSortWorker.reset();

        auto discardedThread = filterSortThread;
        discardedThread->disconnect();
        discardedObjects.append(discardedThread);
        filterSortThread.reset();

        connect(discardedThread.data(), &QThread::finished, this, [this, discardedWorker, discardedThread] {
            discardedObjects.removeAll(discardedWorker);
            discardedObjects.removeAll(discardedThread);
            discardedThread->disconnect();
        },
                Qt::QueuedConnection);

        discardedThread->quit();
    }
}

void FileViewModel::changeState(ModelState newState)
{
    if (state == newState)
        return;

    state = newState;
    Q_EMIT stateChanged();
}

void FileViewModel::closeCursorTimer()
{
    waitTimer.stop();

    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
}

void FileViewModel::startCursorTimer()
{
    if (!waitTimer.isActive())
        waitTimer.start();

    onSetCursorWait();
}
