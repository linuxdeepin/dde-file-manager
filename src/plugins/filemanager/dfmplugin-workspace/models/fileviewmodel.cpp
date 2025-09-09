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
#include "events/workspaceeventcaller.h"

// Include grouping management
#include "utils/groupingmanager.h"

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
#include <dfm-base/utils/protocolutils.h>

#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QPointer>
#include <QList>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

inline constexpr char kDdeFileManager[] { "dde-fileManager" };

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel(parent)

{
    currentKey = QString::number(quintptr(this), 16);
    itemRootData = new FileItemData(dirRootUrl);

    // GroupingManager will be initialized when dirRootUrl is set in initFilterSortWork

    connect(ThumbnailFactory::instance(), &ThumbnailFactory::produceFinished, this, &FileViewModel::onFileThumbUpdated);
    connect(Application::instance(), &Application::genericAttributeChanged, this, &FileViewModel::onGenericAttributeChanged);
    connect(Application::instance(), &Application::showedHiddenFilesChanged, this, &FileViewModel::onHiddenSettingChanged);
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &FileViewModel::onDConfigChanged);
    connect(&waitTimer, &QTimer::timeout, this, &FileViewModel::onSetCursorWait);
    waitTimer.setInterval(50);

    fmDebug() << "FileViewModel initialized with key:" << currentKey;
}

FileViewModel::~FileViewModel()
{
    fmDebug() << "FileViewModel destructor started for key:" << currentKey;
    closeCursorTimer();
    quitFilterSortWork();

    // 清理废弃的对象，确保没有资源泄漏
    for (auto obj : discardedObjects) {
        if (auto thread = qobject_cast<QThread *>(obj.data())) {
            if (thread->isRunning()) {
                thread->quit();
                if (!thread->wait(1000)) {
                    fmWarning() << "Force terminating discarded thread in destructor";
                    thread->terminate();
                    thread->wait(500);
                }
            }
        }
    }
    discardedObjects.clear();

    if (itemRootData) {
        delete itemRootData;
        itemRootData = nullptr;
    }
    FileDataManager::instance()->cleanRoot(dirRootUrl, currentKey);
    fmInfo() << "FileViewModel destructor completed for key:" << currentKey;
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

    FileItemDataPointer itemData = nullptr;
    if (!isParentValid) {
        itemData = filterSortWorker->rootData();
    } else {
        itemData = filterSortWorker->childData(row);
    }

    return createIndex(row, column, itemData.data());
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
        return createIndex(0, 0, data.data());
    } else {
        return QModelIndex();
    }
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    if (!url.isValid()) {
        fmWarning() << "Attempt to set invalid root URL";
        return QModelIndex();
    }

    fmInfo() << "Setting root URL:" << url.toString() << "with strategy:" << static_cast<int>(dirLoadStrategy);

    QVariantMap data;
    data.insert("action", "Open");
    WorkspaceEventCaller::sendEnterDirReportLog(data);

    setDirectoryLoadStrategy(WorkspaceHelper::instance()->getLoadStrategy(url.scheme()));

    // 检查是否需要使用保留策略处理
    bool usePreserveStrategy = !filterSortWorker.isNull() && dirLoadStrategy == DirectoryLoadStrategy::kPreserve;

    if (usePreserveStrategy) {
        fmDebug() << "Using preserve strategy for URL:" << url.toString();
        // 仅准备URL，而不立即加载，这样可以保留当前视图内容
        prepareUrl(url);

        // 如果有预处理器，处理它
        if (WorkspaceHelper::instance()->haveViewRoutePrehandler(url.scheme())) {
            auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
            if (prehandler) {
                fmDebug() << "Executing prehandler for scheme:" << url.scheme();
                quint64 winId = FileManagerWindowsManager::instance().findWindowId(qobject_cast<FileView *>(QObject::parent()));
                prehandler(winId, url, [this]() {
                    // 预处理完成后执行加载
                    this->executeLoad();
                });
                return rootIndex();   // 返回当前索引，保持UI状态
            }
        }

        // 执行加载，但保留当前视图状态
        executeLoad();
        return rootIndex();
    }

    // 标准的创建新模型流程 - kCreateNew策略
    fmDebug() << "Using create new strategy for URL:" << url.toString();
    beginResetModel();
    closeCursorTimer();
    discardFilterSortObjects();   // 确保清理旧的资源

    // create root by url
    dirRootUrl = url;
    FileDataManager::instance()->fetchRoot(dirRootUrl);
    endResetModel();

    initFilterSortWork();

    // fetch files
    const QModelIndex &index = rootIndex();

    if (WorkspaceHelper::instance()->haveViewRoutePrehandler(url.scheme())) {
        auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
        if (prehandler) {
            fmDebug() << "Executing prehandler for scheme:" << url.scheme();
            quint64 winId = FileManagerWindowsManager::instance().findWindowId(qobject_cast<FileView *>(QObject::parent()));
            prehandler(winId, url, [this, index, url]() {
                this->canFetchFiles = true;
                this->fetchingUrl = url;
                this->fetchMore(index);
            });
        }
    } else {
        canFetchFiles = true;
        fetchingUrl = dirRootUrl;
        fetchMore(index);
    }

    fmInfo() << "Root URL set successfully:" << url.toString();
    return index;
}

void FileViewModel::doExpand(const QModelIndex &index)
{
    if (!index.isValid()) {
        fmWarning() << "Attempt to expand invalid index";
        return;
    }

    if (!filterSortWorker) {
        fmWarning() << "The model do not stand by, beacuse the sort worker unexist.";
        return;
    }

    const QUrl &url = index.data(kItemUrlRole).toUrl();
    fmInfo() << "Expanding item:" << url.toString();

    RootInfo *expandRoot = FileDataManager::instance()->fetchRoot(url);

    connect(
            expandRoot, &RootInfo::requestCloseTab, this, [](const QUrl &url) { WorkspaceHelper::instance()->closeTab(url); }, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::getSourceData, expandRoot, &RootInfo::handleGetSourceData, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::sourceDatas, filterSortWorker.data(), &FileSortWorker::handleSourceChildren, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::iteratorLocalFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorLocalChildren, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::iteratorAddFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorChildren, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::iteratorUpdateFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorChildrenUpdate, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::watcherAddFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherAddChildren, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::watcherRemoveFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherRemoveChildren, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::watcherUpdateFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateFile, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::watcherUpdateFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateFiles, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::watcherUpdateHideFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateHideFile, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::traversalFinished, filterSortWorker.data(), &FileSortWorker::handleTraversalFinish, Qt::QueuedConnection);
    connect(expandRoot, &RootInfo::requestSort, filterSortWorker.data(), &FileSortWorker::handleSortDir, Qt::QueuedConnection);

    canFetchFiles = true;
    fetchingUrl = url;
    fetchMore(index);

    FileItemDataPointer item = filterSortWorker->childData(index.row());
    if (item)
        item->setExpanded(true);
}

void FileViewModel::doCollapse(const QModelIndex &index)
{
    if (!index.isValid()) {
        fmWarning() << "Attempt to collapse invalid index";
        return;
    }

    const QUrl &collapseUrl = index.data(kItemUrlRole).toUrl();
    fmInfo() << "Collapsing item:" << collapseUrl.toString();

    Q_EMIT requestCollapseItem(currentKey, collapseUrl);

    FileItemDataPointer item = filterSortWorker->childData(index.row());
    if (item && item->data(Global::ItemRoles::kItemTreeViewExpandedRole).toBool()) {
        item->setExpanded(false);
        FileDataManager::instance()->cleanRoot(collapseUrl, currentKey);
        Q_EMIT dataChanged(index, index);
    }
}

FileInfoPointer FileViewModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || filterSortWorker.isNull())
        return nullptr;

    const QModelIndex &parentIndex = index.parent();
    FileItemDataPointer item { nullptr };
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

    FileItemDataPointer itemData = nullptr;
    int columnRole = role;
    if (!parentIndex.isValid()) {   // !parentIndex.isValid() 意味着这是目录本身
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

    if (!itemData) {
        fmWarning() << "Failed to get data for index:" << index << "role:" << role << "URL:" << dirRootUrl.toString();
        return QVariant();
    }

    if (itemData->data(ItemRoles::kItemUrlRole).toUrl().scheme() == "group-header") {
        QVariant groupHeaderVaule = filterSortWorker->groupHeaderData(index.row(), role);
        if (groupHeaderVaule.isValid())
            return groupHeaderVaule;
    }

    return itemData->data(columnRole);
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
    fmInfo() << "Refreshing view for URL:" << dirRootUrl.toString();
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
        fmDebug() << "Cannot fetch more data for URL:" << fetchingUrl.toString();
        return;
    }
    canFetchFiles = false;

    bool ret { false };

    if (!fetchingUrl.isValid()) {
        fmWarning() << "Can't fetch more with invalid url:" << fetchingUrl.toString();
        return;
    }

    fmDebug() << "Starting to fetch files for URL:" << fetchingUrl.toString();

    if (filterSortWorker.isNull()) {
        fmDebug() << "Using direct fetch mode for URL:" << fetchingUrl.toString();
        ret = FileDataManager::instance()->fetchFiles(fetchingUrl, currentKey);
    } else {
        fmDebug() << "Using filter sort worker fetch mode for URL:" << fetchingUrl.toString();
        ret = FileDataManager::instance()->fetchFiles(fetchingUrl,
                                                      currentKey,
                                                      filterSortWorker->getSortRole(),
                                                      filterSortWorker->getSortOrder());
    }

    if (ret) {
        fmDebug() << "File fetch request sent successfully for URL:" << fetchingUrl.toString();
        changeState(ModelState::kBusy);
        startCursorTimer();
    } else {
        fmWarning() << "Failed to fetch files for URL:" << fetchingUrl.toString() << "currentKey:" << currentKey;
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

    if (index.data(kItemIsGroupHeaderType).toBool()) {
        return Qt::ItemIsEnabled;
    }

    if (index.data(kItemFileCanRenameRole).toBool())
        flags |= Qt::ItemIsEditable;

    if (index.data(kItemFileCanDropRole).toBool())
        flags |= Qt::ItemIsDropEnabled;

    // use can drop attribute,if error modify the fileinfo candrop attribute
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
            const QUrl &url = (*it).data(Global::ItemRoles::kItemUrlRole).toUrl();
            if (urlsSet.contains(url))
                continue;

            urls << url;
            urlsSet << url;
        }
    }

    QMimeData *data = new QMimeData();
    data->setText(kDdeFileManager);
    data->setUrls(urls);
    SysInfoUtils::setMimeDataUserId(data);
    data->setData(DFMGLOBAL_NAMESPACE::Mime::kDFMAppTypeKey, QByteArray(kDdeFileManager));

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
        fmWarning() << "Drop target dir is not writable!";
        return false;
    }
    QUrl targetUrl = targetFileInfo->urlOf(UrlInfoType::kUrl);
    const QList<QUrl> &dropUrls = data->urls();

    if (targetFileInfo->isAttributes(OptInfoType::kIsSymLink))
        targetUrl = QUrl::fromLocalFile(targetFileInfo->pathOf(PathInfoType::kSymLinkTarget));

    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());

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

    if (FileUtils::isTrashDesktopFile(targetUrl)) {
        FileOperatorHelperIns->moveToTrash(view, treeSelectUrl.isEmpty() ? dropUrls : treeSelectUrl);
        return true;
    } else if (FileUtils::isDesktopFileSuffix(targetUrl)) {
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
            FileOperatorHelperIns->dropFiles(view, action, targetUrl, treeSelectUrl.isEmpty() ? dropUrls : treeSelectUrl);
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
    if (state == ModelState::kBusy) {
        fmWarning() << "Cannot sort while model is busy for URL:" << dirRootUrl.toString();
        return;
    }

    ItemRoles role = getRoleByColumn(column);
    fmInfo() << "Sorting by column:" << column << "role:" << role << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending") << "URL:" << dirRootUrl.toString();

    Q_EMIT requestSortChildren(order, role,
                               Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool());
}

void FileViewModel::stopTraversWork(const QUrl &newUrl)
{
    fmInfo() << "Stopping traversal work, current URL:" << dirRootUrl.toString() << "new URL:" << newUrl.toString();

    changeState(ModelState::kIdle);
    closeCursorTimer();

    bool canUsePreserveStrategy = dirRootUrl.isValid() && (newUrl.scheme() == dirRootUrl.scheme());
    if (dirLoadStrategy == DirectoryLoadStrategy::kPreserve && canUsePreserveStrategy) {
        fmDebug() << "Using preserve strategy to stop work";
        // stop work but do not clean current data
        FileDataManager::instance()->stopRootWork(dirRootUrl, currentKey);
        FileDataManager::instance()->cleanUnusedRoots(dirRootUrl, currentKey);
        return;
    }

    fmDebug() << "Cleaning all data due to strategy or scheme change";
    discardFilterSortObjects();
    FileDataManager::instance()->cleanRoot(dirRootUrl, currentKey);
}

QList<ItemRoles> FileViewModel::getColumnRoles() const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(dirRootUrl, &roles);

    QVariant headerListValue = WorkspaceHelper::instance()->getFileViewStateValue(dirRootUrl, "headerList");
    if (!headerListValue.isNull()) {
        QVariantList headerList = headerListValue.toList();

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
                                                                           << kItemFileCreatedRole
                                                                           << kItemFileSizeRole
                                                                           << kItemFileMimeTypeRole;

        int customCount = roles.count();
        for (auto role : defualtColumnRoleList) {
            if (!roles.contains(role)) {
                int index = roles.length() - customCount;

                if (index > roles.size() || index < 0)
                    roles.append(role);
                else
                    roles.insert(index, role);
            }
        }
    }

    return roles;
}

ItemRoles FileViewModel::columnToRole(int column) const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(dirRootUrl, &roles);

    QVariant headerListValue = WorkspaceHelper::instance()->getFileViewStateValue(dirRootUrl, "headerList");
    if (!headerListValue.isNull()) {
        QVariantList headerList = headerListValue.toList();
        if (headerList.length() > column)
            return ItemRoles(headerList.at(column).toInt());

    } else if (!customOnly) {
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                                           << kItemFileLastModifiedRole
                                                                           << kItemFileCreatedRole
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
    case kItemFileCreatedRole:
        return tr("Time created");
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
    fmDebug() << "Setting filters:" << filters << "for URL:" << dirRootUrl.toString();
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

    fmDebug() << "Setting name filters:" << filters << "for URL:" << dirRootUrl.toString();
    nameFilters = filters;
    Q_EMIT requestChangeNameFilters(filters);
}

QStringList FileViewModel::getNameFilters() const
{
    return nameFilters;
}

void FileViewModel::setFilterData(const QVariant &data)
{
    fmDebug() << "Setting filter data for URL:" << dirRootUrl.toString();
    filterData = data;
    // 设置要触发重新过滤信号
    Q_EMIT requestSetFilterData(data);
}

void FileViewModel::setFilterCallback(const FileViewFilterCallback callback)
{
    fmDebug() << "Setting filter callback for URL:" << dirRootUrl.toString();
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
    if (!info) {
        fmWarning() << "Cannot update thumbnail: file info is null for index:" << index;
        return;
    }

    // Creating thumbnail icon in a thread may cause the program to crash
    QIcon thumbIcon(thumb);
    if (thumbIcon.isNull()) {
        fmWarning() << "Cannot update thumbnail: icon is null for thumb:" << thumb;
        return;
    }

    fmDebug() << "Updating thumbnail for file:" << info->urlOf(UrlInfoType::kUrl).toString();
    info->setExtendedAttributes(ExtInfoType::kFileThumbnail, thumbIcon);
}

void FileViewModel::setTreeView(const bool isTree)
{
    this->isTree = isTree;
    Q_EMIT requestTreeView(isTree);
}

QStringList FileViewModel::getKeyWords()
{
    auto rootInfo = FileDataManager::instance()->fetchRoot(dirRootUrl);
    if (rootInfo)
        return rootInfo->getKeyWords();

    return {};
}

void FileViewModel::setDirectoryLoadStrategy(DirectoryLoadStrategy strategy)
{
    fmDebug() << "Setting directory load strategy:" << static_cast<int>(strategy) << "for URL:" << dirRootUrl.toString();
    dirLoadStrategy = strategy;
}

DirectoryLoadStrategy FileViewModel::directoryLoadStrategy() const
{
    return dirLoadStrategy;
}

// 准备URL但不执行加载
void FileViewModel::prepareUrl(const QUrl &url)
{
    if (!url.isValid()) {
        fmWarning() << "Cannot prepare invalid URL";
        return;
    }

    fmDebug() << "Preparing URL:" << url.toString() << "with strategy:" << static_cast<int>(dirLoadStrategy);
    preparedUrl = url;
}

// 执行实际的加载
void FileViewModel::executeLoad()
{
    if (filterSortWorker.isNull()) {
        // 没有工作线程时，直接创建新的
        if (preparedUrl.isValid()) {
            fmDebug() << "No filter sort worker, creating new model for URL:" << preparedUrl.toString();
            setRootUrl(preparedUrl);
            preparedUrl = QUrl();
        }
        return;
    }

    // 使用准备好的URL，如果没有设置则使用当前URL
    QUrl urlToLoad = preparedUrl.isValid() ? preparedUrl : dirRootUrl;
    if (!urlToLoad.isValid()) {
        fmWarning() << "Cannot execute load: no valid URL to load";
        return;
    }

    fmDebug() << "Executing load for URL:" << urlToLoad.toString() << "with strategy:" << static_cast<int>(dirLoadStrategy);

    // 根据不同策略处理
    switch (dirLoadStrategy) {
    case DirectoryLoadStrategy::kCreateNew:
        // 创建新策略：直接设置新URL，会立即清空旧视图
        fmDebug() << "Using create new strategy for URL:" << urlToLoad.toString();
        setRootUrl(urlToLoad);
        break;

    case DirectoryLoadStrategy::kPreserve: {
        // 保留策略：保留当前视图内容直到新数据加载完成
        fmDebug() << "Using preserve strategy for URL:" << urlToLoad.toString();

        // 更新当前URL（但不影响视图显示）
        dirRootUrl = urlToLoad;

        // 获取目标URL的RootInfo，准备数据获取
        RootInfo *newRoot = FileDataManager::instance()->fetchRoot(dirRootUrl);
        newRoot->setFirstBatch(true);

        // 连接信号，使当前filterSortWorker监听新RootInfo的数据
        connectRootAndFilterSortWork(newRoot, true);

        // 更新状态为获取中
        changeState(ModelState::kBusy);

        // 触发数据加载过程
        const QModelIndex &index = rootIndex();
        canFetchFiles = true;
        fetchingUrl = urlToLoad;
        fetchMore(index);   // 这是关键 - 触发目录数据迭代逻辑

        break;
    }
    }

    // 清除准备的URL
    preparedUrl = QUrl();
    fmDebug() << "Load execution completed for URL:" << urlToLoad.toString();
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

    if (filterSortWorker && filterSortWorker->childrenCount() <= 0 && UniversalUtils::urlEquals(rootUrl(), FileUtils::trashRootUrl()))
        WorkspaceEventCaller::sendModelFilesEmpty();
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

    if (DConfigInfo::kMtpThumbnailKey == key && ProtocolUtils::isMTPFile(rootUrl()))
        Q_EMIT requestClearThumbnail();
}

void FileViewModel::onSetCursorWait()
{
    if (QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::CursorShape::WaitCursor)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void FileViewModel::onHiddenSettingChanged(bool value)
{
    if (value) {
        currentFilters |= QDir::Hidden;
    } else {
        currentFilters &= ~QDir::Hidden;
    }
    Q_EMIT requestShowHiddenChanged(value);
}

void FileViewModel::onWorkFinish(int visiableCount, int totalCount)
{
    fmInfo() << "Work finished for URL:" << dirRootUrl.toString()
             << "visible files:" << visiableCount << "total files:" << totalCount;

    QVariantMap data;
    data.insert("action", "Finish");
    data.insert("visiable files", visiableCount);
    data.insert("total files", totalCount);
    WorkspaceEventCaller::sendEnterDirReportLog(data);

    this->changeState(ModelState::kIdle);
    closeCursorTimer();

    // 如果是保留策略，在加载完成后清理旧的RootInfo对象
    if (dirLoadStrategy == DirectoryLoadStrategy::kPreserve) {
        fmDebug() << "Cleaning unused roots after preserve strategy completion for URL:" << dirRootUrl.toString();
        // 获取当前URL所有子目录的RootInfo
        FileDataManager::instance()->cleanUnusedRoots(dirRootUrl, currentKey);
    }
}

void FileViewModel::onDataChanged(int first, int last)
{
    QModelIndex firstIndex = index(first, 0, rootIndex());
    QModelIndex lastIndex = index(last, 0, rootIndex());

    Q_EMIT dataChanged(firstIndex, lastIndex);
}

void FileViewModel::connectRootAndFilterSortWork(RootInfo *root, const bool refresh)
{
    if (filterSortWorker.isNull()) {
        fmWarning() << "Cannot connect root and filter sort work: filter sort worker is null";
        return;
    }

    if (refresh) {
        auto token = QString::number(quintptr(filterSortWorker.data()), 16);
        if (root->connectTokens().contains(token)) {
            fmDebug() << "Root already connected with token:" << token;
            return;
        }
        root->addConnectToken(token);
        fmDebug() << "Added connection token:" << token << "for root URL:" << rootUrl();
    }

    connect(
            root, &RootInfo::requestCloseTab, this, [](const QUrl &url) { WorkspaceHelper::instance()->closeTab(url); }, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::getSourceData, root, &RootInfo::handleGetSourceData, Qt::QueuedConnection);
    connect(root, &RootInfo::sourceDatas, filterSortWorker.data(), &FileSortWorker::handleSourceChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::iteratorLocalFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorLocalChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::iteratorAddFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::iteratorUpdateFiles, filterSortWorker.data(), &FileSortWorker::handleIteratorChildrenUpdate, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherAddFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherAddChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherRemoveFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherRemoveChildren, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherUpdateFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateFile, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherUpdateFiles, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateFiles, Qt::QueuedConnection);
    connect(root, &RootInfo::watcherUpdateHideFile, filterSortWorker.data(), &FileSortWorker::handleWatcherUpdateHideFile, Qt::QueuedConnection);
    connect(root, &RootInfo::traversalFinished, filterSortWorker.data(), &FileSortWorker::handleTraversalFinish, Qt::QueuedConnection);
    connect(root, &RootInfo::requestSort, filterSortWorker.data(), &FileSortWorker::handleSortDir, Qt::QueuedConnection);

    connect(root, &RootInfo::renameFileProcessStarted, this, &FileViewModel::renameFileProcessStarted);

    fmDebug() << "Root and filter sort work connected successfully for URL:" << rootUrl();
}

void FileViewModel::initFilterSortWork()
{
    fmDebug() << "Initializing filter sort work for URL:" << dirRootUrl.toString();

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
        fmDebug() << "Set default filters:" << currentFilters << "for URL:" << dirRootUrl.toString();
    }

    // get sort config
    Qt::SortOrder order = static_cast<Qt::SortOrder>(WorkspaceHelper::instance()->getFileViewStateValue(dirRootUrl, "sortOrder", Qt::SortOrder::AscendingOrder).toInt());
    ItemRoles role = static_cast<ItemRoles>(WorkspaceHelper::instance()->getFileViewStateValue(dirRootUrl, "sortRole", kItemFileDisplayNameRole).toInt());

    fmDebug() << "Sort configuration - order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending") << "role:" << role << "for URL:" << dirRootUrl.toString();

    if (filterSortWorker)
        filterSortWorker->disconnect();

    filterSortWorker.reset(new FileSortWorker(dirRootUrl, currentKey, filterCallback, nameFilters, currentFilters));
    beginInsertRows(QModelIndex(), 0, 0);
    auto rootInfo = InfoFactory::create<FileInfo>(dirRootUrl);
    if (!rootInfo.isNull()) {
        rootInfo->updateAttributes();
        fmDebug() << "Root info created and attributes updated for URL:" << dirRootUrl.toString();
    }
    filterSortWorker->setRootData(FileItemDataPointer(new FileItemData(dirRootUrl, rootInfo)));
    endInsertRows();
    filterSortWorker->setSortAgruments(order, role, Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool());

    filterSortWorker->setTreeView(this->isTree
                                  && WorkspaceHelper::instance()->isViewModeSupported(rootUrl().scheme(), ViewMode::kTreeMode));
    filterSortWorker->moveToThread(filterSortThread.data());

    // 连接信号
    connectFilterSortWorkSignals();

    RootInfo *root = FileDataManager::instance()->fetchRoot(dirRootUrl);
    connectRootAndFilterSortWork(root);

    // Initialize and restore grouping settings
    initializeGroupingManager();
    m_groupingManager->restoreSettings();
    m_groupingManager->applyToWorker(filterSortWorker);

    filterSortThread->start();
    fmInfo() << "Filter sort work initialized and started for URL:" << dirRootUrl.toString();
}

void FileViewModel::quitFilterSortWork()
{
    fmDebug() << "Quitting filter sort work for URL:" << dirRootUrl.toString();

    if (!filterSortWorker.isNull()) {
        filterSortWorker->disconnect();
        filterSortWorker->cancel();
        fmDebug() << "Filter sort worker disconnected and cancelled";
    }
    if (!filterSortThread.isNull()) {
        filterSortThread->quit();

        // 等待线程优雅退出，增加超时处理
        if (!filterSortThread->wait(3000)) {
            fmWarning() << "FilterSortThread did not exit within 3 seconds, forcing termination for URL:" << dirRootUrl.toString();
            filterSortThread->terminate();
            if (!filterSortThread->wait(1000)) {
                fmWarning() << "FilterSortThread termination failed, potential resource leak for URL:" << dirRootUrl.toString();
            }
        }
    }

    // 确保智能指针被重置
    filterSortWorker.reset();
    filterSortThread.reset();
    fmDebug() << "Filter sort work quit completed for URL:" << dirRootUrl.toString();
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

        connect(
                discardedThread.data(), &QThread::finished, this, [this, discardedWorker, discardedThread] {
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

void FileViewModel::connectFilterSortWorkSignals()
{
    if (filterSortWorker.isNull())
        return;

    // connect signals
    connect(filterSortWorker.data(), &FileSortWorker::insertRows, this, &FileViewModel::onInsert, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::insertFinish, this, &FileViewModel::onInsertFinish, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::removeRows, this, &FileViewModel::onRemove, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::removeFinish, this, &FileViewModel::onRemoveFinish, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::dataChanged, this, &FileViewModel::onDataChanged, Qt::QueuedConnection);
    connect(
            filterSortWorker.data(), &FileSortWorker::requestFetchMore, this, [this]() {
        canFetchFiles = true;
        fetchingUrl = rootUrl();
        RootInfo *root = FileDataManager::instance()->fetchRoot(dirRootUrl);
        connectRootAndFilterSortWork(root, true);
        fetchMore(rootIndex()); }, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::updateRow, this, &FileViewModel::onFileUpdated, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::selectAndEditFile, this, &FileViewModel::selectAndEditFile, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::requestSetIdel, this, &FileViewModel::onWorkFinish, Qt::QueuedConnection);
    connect(
            filterSortWorker.data(), &FileSortWorker::requestCursorWait, this, [this] {
                startCursorTimer();
            },
            Qt::QueuedConnection);
    connect(
            filterSortWorker.data(), &FileSortWorker::reqUestCloseCursor, this, [this] {
                closeCursorTimer();
            },
            Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeHiddenFilter, filterSortWorker.data(), &FileSortWorker::onToggleHiddenFiles, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeFilters, filterSortWorker.data(), &FileSortWorker::handleFilters, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestChangeNameFilters, filterSortWorker.data(), &FileSortWorker::HandleNameFilters, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestUpdateFile, filterSortWorker.data(), &FileSortWorker::handleUpdateFile, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestSortChildren, filterSortWorker.data(), &FileSortWorker::handleResort, Qt::QueuedConnection);

    connect(this, &FileViewModel::requestSetFilterData, filterSortWorker.data(), &FileSortWorker::handleFilterData, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestSetFilterCallback, filterSortWorker.data(), &FileSortWorker::handleFilterCallFunc, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestRefreshAllChildren, filterSortWorker.data(), &FileSortWorker::handleRefresh, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestClearThumbnail, filterSortWorker.data(), &FileSortWorker::handleClearThumbnail, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestShowHiddenChanged, filterSortWorker.data(), &FileSortWorker::onShowHiddenFileChanged, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestCollapseItem, filterSortWorker.data(), &FileSortWorker::handleCloseExpand, Qt::QueuedConnection);
    connect(this, &FileViewModel::requestTreeView, filterSortWorker.data(), &FileSortWorker::handleSwitchTreeView, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::requestUpdateView, this, &FileViewModel::onUpdateView, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::aboutToSwitchToListView, this, &FileViewModel::aboutToSwitchToListView, Qt::QueuedConnection);
    connect(filterSortWorker.data(), &FileSortWorker::groupingDataChanged, this, &FileViewModel::onGroupingDataChanged, Qt::QueuedConnection);
    connect(Application::instance(), &Application::appAttributeChanged, filterSortWorker.data(), &FileSortWorker::onAppAttributeChanged, Qt::QueuedConnection);
}

// Grouping management methods (delegated to GroupingManager)
void FileViewModel::setGroupingStrategy(const QString &strategyName)
{
    if (m_groupingManager) {
        m_groupingManager->setGroupingStrategy(strategyName);
        // Apply to worker if available
        if (!filterSortWorker.isNull()) {
            m_groupingManager->applyToWorker(filterSortWorker);
        }
    }
}

QString FileViewModel::getGroupingStrategy() const
{
    return m_groupingManager ? m_groupingManager->getGroupingStrategy() : QString("NoGroupStrategy");
}

void FileViewModel::setGroupingEnabled(bool enabled)
{
    if (m_groupingManager) {
        m_groupingManager->setGroupingEnabled(enabled);
        // Apply to worker if available
        if (!filterSortWorker.isNull()) {
            m_groupingManager->applyToWorker(filterSortWorker);
        }
    }
}

bool FileViewModel::isGroupingEnabled() const
{
    return m_groupingManager ? m_groupingManager->isGroupingEnabled() : false;
}

void FileViewModel::setGroupingOrder(Qt::SortOrder order)
{
    if (m_groupingManager) {
        m_groupingManager->setGroupingOrder(order);
        // Apply to worker if available
        if (!filterSortWorker.isNull()) {
            m_groupingManager->applyToWorker(filterSortWorker);
        }
    }
}

Qt::SortOrder FileViewModel::getGroupingOrder() const
{
    return m_groupingManager ? m_groupingManager->getGroupingOrder() : Qt::AscendingOrder;
}

void FileViewModel::toggleGroupExpansion(const QString &groupKey)
{
    if (m_groupingManager) {
        m_groupingManager->toggleGroupExpansion(groupKey);
        // Apply to worker if available
        if (!filterSortWorker.isNull()) {
            filterSortWorker->toggleGroupExpansion(groupKey);
        }
    }
}

bool FileViewModel::isGroupExpanded(const QString &groupKey) const
{
    return m_groupingManager ? m_groupingManager->isGroupExpanded(groupKey) : true;
}

void FileViewModel::initializeGroupingManager()
{
    if (!m_groupingManager || dirRootUrl.isValid()) {
        // Create or recreate GroupingManager for current URL
        m_groupingManager.reset(new GroupingManager(dirRootUrl, this));

        // Connect signals
        connect(m_groupingManager.get(), &GroupingManager::groupingStrategyChanged,
                this, &FileViewModel::groupingStrategyChanged);
        connect(m_groupingManager.get(), &GroupingManager::groupingEnabledChanged,
                this, &FileViewModel::groupingEnabledChanged);
        connect(m_groupingManager.get(), &GroupingManager::groupingOrderChanged,
                this, &FileViewModel::groupingOrderChanged);
        connect(m_groupingManager.get(), &GroupingManager::groupExpansionChanged,
                this, &FileViewModel::groupExpansionChanged);

        fmDebug() << "GroupingManager initialized for URL:" << dirRootUrl.toString();
    }
}

void FileViewModel::onGroupingDataChanged()
{
    fmDebug() << "Grouping data changed - refreshing model view for URL:" << dirRootUrl.toString();

    if (filterSortWorker.isNull()) {
        fmWarning() << "Cannot handle grouping data change: worker is null";
        return;
    }

    // Get the new item count after regrouping
    int newRowCount = filterSortWorker->childrenCount();

    fmInfo() << "Model data updated after regrouping - new row count:" << newRowCount << "for URL:" << dirRootUrl.toString();

    if (newRowCount > 0)
        onDataChanged(0, newRowCount - 1);

    fmDebug() << "Model reset completed after grouping data change";
}
