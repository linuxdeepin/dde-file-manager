// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesortworker.h"
#include "workspacehelper.h"
#include "groups/groupingfactory.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/sortutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QStandardPaths>

#include <sys/stat.h>

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;
using namespace dfmio;

namespace {
template<class T>
void insertToList(QList<T> &list, int index, const T &t)
{
    if (index < 0 || index > list.size())
        list.append(t);
    else
        list.insert(index, t);
}
}   // namespace

FileSortWorker::FileSortWorker(const QUrl &url, const QString &key, FileViewFilterCallback callfun,
                               const QStringList &nameFilters, const QDir::Filters filters, QObject *parent)
    : QObject(parent),
      current(url),
      nameFilters(nameFilters),
      filters(filters),
      filterCallback(callfun),
      currentKey(key)
{
    fmDebug() << "FileSortWorker created for URL:" << url.toString() << "key:" << key;

    auto dirPath = url.path();
    if (!dirPath.isEmpty() && dirPath != QDir::separator() && url.path().endsWith(QDir::separator()))
        dirPath.chop(1);
    current.setPath(dirPath);
    isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();

    fmDebug() << "Mixed dir and file sorting enabled:" << isMixDirAndFile;

    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FileSortWorker::handleFileInfoUpdated, Qt::QueuedConnection);
    currentSupportTreeView = WorkspaceHelper::instance()->isViewModeSupported(current.scheme(), ViewMode::kTreeMode);

    fmDebug() << "Tree view supported:" << currentSupportTreeView;

    connect(this, &FileSortWorker::requestSortByMimeType, this, &FileSortWorker::handleSortByMimeType,
            Qt::QueuedConnection);

    // Initialize grouping engine
    groupingEngine = std::make_unique<GroupingEngine>(current, this);
    // Set cancellation callback to allow GroupingEngine to check our cancel state
    groupingEngine->setCancellationCheckCallback([this]() -> bool {
        return this->isCanceled;
    });

    fmDebug() << "FileSortWorker: Grouping engine initialized";
}

FileSortWorker::~FileSortWorker()
{
    fmDebug() << "FileSortWorker destructor called, canceling operations";

    isCanceled = true;

    // 停止定时器（Qt会自动删除有parent的QTimer）
    if (updateRefresh) {
        updateRefresh->stop();
        updateRefresh = nullptr;
    }

    // 清理数据结构
    childrenDataMap.clear();
    visibleChildren.clear();
    children.clear();
    visibleTreeChildren.clear();
    fileInfoRefresh.clear();
    waitUpdatedFiles.clear();
}

FileSortWorker::GroupingOpt FileSortWorker::setGroupArguments(const Qt::SortOrder order,
                                                              const QString &strategy,
                                                              const QVariantHash &expandStates)
{
    fmInfo() << "Setting group arguments - strategy:" << strategy << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
    QString oldStrategy = currentStrategy ? currentStrategy->getStrategyName() : "";
    groupOrder = order;
    currentStrategy = GroupingFactory::createStrategy(strategy, this);
    if (!currentStrategy) {
        return FileSortWorker::GroupingOpt::kGroupingOptNone;
    }
    isCurrentGroupingEnabled = !(currentStrategy->getStrategyName() == GroupStrategy::kNoGroup);
    groupExpansionStates.clear();
    for (const auto &key : expandStates.keys()) {
        groupExpansionStates.insert(key, expandStates.value(key).toBool());
    }

    if (!currentStrategy || !isCurrentGroupingEnabled || !groupingEngine) {
        return FileSortWorker::GroupingOpt::kGroupingOptNone;
    }

    return strategy == oldStrategy ? FileSortWorker::GroupingOpt::kGroupingOptOnlyOrderChanged : FileSortWorker::GroupingOpt::kGroupingOptOtherChanged;
}

FileSortWorker::SortOpt FileSortWorker::setSortArguments(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile)
{
    // 强制树形模式下 isMixDirAndFile 为 false
    bool mixDirAndFile = istree ? false : isMixDirAndFile;

    FileSortWorker::SortOpt opt { FileSortWorker::SortOpt::kSortOptNone };
    if (sortOrder == order && orgSortRole == sortRole && this->isMixDirAndFile == mixDirAndFile)
        return opt;
    if (orgSortRole != sortRole || this->isMixDirAndFile != mixDirAndFile) {
        opt = FileSortWorker::SortOpt::kSortOptOtherChanged;
    } else {
        opt = FileSortWorker::SortOpt::kSortOptOnlyOrderChanged;
    }

    fmInfo() << "Setting sort arguments - order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
             << "role:" << static_cast<int>(sortRole) << "mix dir and file:" << mixDirAndFile;

    sortOrder = order;
    orgSortRole = sortRole;
    this->isMixDirAndFile = mixDirAndFile;
    switch (sortRole) {
    case Global::ItemRoles::kItemFileDisplayNameRole:
        this->sortRole = DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName;
        break;
    case Global::ItemRoles::kItemFileSizeRole:
        this->sortRole = DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileSize;
        break;
    case Global::ItemRoles::kItemFileLastReadRole:
        this->sortRole = DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastRead;
        break;
    case Global::ItemRoles::kItemFileLastModifiedRole:
        this->sortRole = DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastModified;
        break;
    default:
        this->sortRole = DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
    }

    return opt;
}

int FileSortWorker::childrenCount()
{
    if (!isCurrentGroupingEnabled) {
        // Traditional mode: use original logic
        return childrenCountInternal();
    } else {
        // Grouping mode: return flattened item count
        return groupedModelData.getItemCount();
    }
}

int FileSortWorker::getFileItemCount()
{
    if (!isCurrentGroupingEnabled) {
        // Traditional mode: use original logic (same as childrenCount)
        return childrenCountInternal();
    }

    // Grouping mode: return only file items count (excludes group headers)
    return groupedModelData.getFileItemCount();
}

int FileSortWorker::getGroupItemCount()
{
    if (!isCurrentGroupingEnabled) {
        return 0;
    }

    return groupedModelData.getGroupItemCount();
}

QVariant FileSortWorker::groupHeaderData(const int index, const int role)
{
    // Grouping mode: use flattened data mapping
    ModelItemWrapper wrapper = groupedModelData.getItemAt(index);
    if (!wrapper.isValid()) {
        fmDebug() << "Invalid index for groupHeaderData:" << index;
        return {};
    }

    if (wrapper.isFileItem()) {
        return {};
    }

    return wrapper.getData(role);
}

FileItemDataPointer FileSortWorker::childData(const QUrl &url)
{
    QReadLocker lk(&childrenDataLocker);
    return childrenDataMap.value(url);
}

void FileSortWorker::setRootData(const FileItemDataPointer data)
{
    rootdata = data;
}

FileItemDataPointer FileSortWorker::rootData() const
{
    return rootdata;
}

FileItemDataPointer FileSortWorker::childData(const int index)
{
    if (!isCurrentGroupingEnabled) {
        // Traditional mode: use original logic
        QUrl url;
        {
            QReadLocker lk(&locker);
            if (index < 0 || index >= visibleChildren.count()) {
                fmDebug() << "Invalid index for childData:" << index << "visible children count:" << visibleChildren.count();
                return nullptr;
            }
            url = visibleChildren.at(index);
        }

        QReadLocker lk(&childrenDataLocker);
        return childrenDataMap.value(url);
    } else {
        // Grouping mode: use flattened data mapping
        ModelItemWrapper wrapper = groupedModelData.getItemAt(index);
        if (!wrapper.isValid()) {
            fmDebug() << "Invalid index for grouped childData:" << index;
            return nullptr;
        }

        if (wrapper.isGroupHeader() && wrapper.fileData) {
            wrapper.fileData->setParentData(rootdata.data());
        }

        return wrapper.fileData;
    }
}

void FileSortWorker::cancel()
{
    fmDebug() << "Canceling FileSortWorker operations";
    isCanceled = true;
    mimeSorting = false;
}

int FileSortWorker::getChildShowIndex(const QUrl &url)
{
    if (!isCurrentGroupingEnabled) {
        return getChildShowIndexInternal(url);
    } else {
        return groupedModelData.findFileStartPos(url).value_or(-1);
    }
}

QList<QUrl> FileSortWorker::getChildrenUrls()
{
    QReadLocker lk(&locker);
    fmDebug() << "Getting children URLs, count:" << visibleChildren.size();
    return visibleChildren;
}

ItemRoles FileSortWorker::getSortRole() const
{
    return orgSortRole;
}

Qt::SortOrder FileSortWorker::getSortOrder() const
{
    return sortOrder;
}

QString FileSortWorker::getGroupStrategyName() const
{
    if (!currentStrategy) {
        return GroupStrategy::kNoGroup;
    }

    return currentStrategy->getStrategyName();
}

Qt::SortOrder FileSortWorker::getGroupOrder() const
{
    return groupOrder;
}

void FileSortWorker::setTreeView(const bool isTree)
{
    fmInfo() << "Setting tree view mode:" << isTree << "current mode:" << istree;
    istree = isTree;
    isMixDirAndFile = istree ? false : isMixDirAndFile;
    fmDebug() << "Mixed dir and file sorting now:" << isMixDirAndFile;
}

bool FileSortWorker::currentIsGroupingMode() const
{
    return isCurrentGroupingEnabled;
}

void FileSortWorker::handleIteratorLocalChildren(const QString &key,
                                                 const QList<SortInfoPointer> children,
                                                 const DEnumerator::SortRoleCompareFlag sortRole,
                                                 const Qt::SortOrder sortOrder,
                                                 const bool isMixDirAndFile,
                                                 bool isFirstBatch)
{
    fmDebug() << "Handling iterator local children - key:" << key << "children count:" << children.size() << "first batch:" << isFirstBatch;
    // This is where we handle the first batch flag for kPreserve mode
    handleAddChildren(key, children, {}, sortRole, sortOrder, isMixDirAndFile, false, false, true, isFirstBatch);
}

void FileSortWorker::handleSourceChildren(const QString &key,
                                          const QList<SortInfoPointer> children,
                                          const DEnumerator::SortRoleCompareFlag sortRole,
                                          const Qt::SortOrder sortOrder, const bool isMixDirAndFile,
                                          const bool isFinished)
{
    // Source changes are not the first batch
    handleAddChildren(key, children, {}, sortRole, sortOrder, isMixDirAndFile, true, isFinished, true, false);
}

void FileSortWorker::handleIteratorChildren(const QString &key, const QList<SortInfoPointer> children, const QList<FileInfoPointer> infos, bool isFirstBatch)
{
    // The isFirstBatch parameter is only passed through but not used for clearing
    // since this is not part of the logic path that should use it
    handleAddChildren(key, children, infos, sortRole, sortOrder, isMixDirAndFile, false, false, false, isFirstBatch);
}

void FileSortWorker::handleIteratorChildrenUpdate(const QString &key, const QList<SortInfoPointer> children, bool isFirstBatch)
{
    if (key != currentKey || isCanceled)
        return;

    QList<SortInfoPointer> newChildren {};
    for (const auto &sortInfo : children) {
        if (!sortInfo)
            continue;

        QUrl fileUrl = sortInfo->fileUrl();

        // 现在加写锁
        QWriteLocker lk(&childrenDataLocker);
        if (childrenDataMap.contains(fileUrl)) {
            // 更新已存在的文件信息的排序信息
            auto itemData = childrenDataMap.value(fileUrl);
            if (itemData)
                itemData->setSortFileInfo(sortInfo);
        } else {
            newChildren.append(sortInfo);
        }
    }

    // Pass false for isFirstBatch since these are updates, not initial data
    handleAddChildren(key, newChildren, {}, isFirstBatch);
}

void FileSortWorker::handleTraversalFinish(const QString &key, bool noDataProduced)
{
    if (currentKey != key) {
        fmDebug() << "Ignoring traversal finish for different key - current:" << currentKey << "received:" << key;
        return;
    }

    fmInfo() << "Traversal finished - no data produced:" << noDataProduced << "visible count:" << visibleChildren.count() << "total count:" << childrenDataMap.count();

    // If no data was produced during traversal, clear the existing data
    if (noDataProduced) {
        fmDebug() << "Clearing data due to no data produced during traversal";
        visibleTreeChildren.clear();

        QWriteLocker childLock(&childrenDataLocker);
        childrenDataMap.clear();

        QWriteLocker visLock(&locker);
        visibleChildren.clear();

        children.clear();
    }

    Q_EMIT requestSetIdel(visibleChildren.count(), childrenDataMap.count());

    HandleNameFilters(nameFilters);
}

void FileSortWorker::handleSortDir(const QString &key, const QUrl &parent)
{
    if (currentKey != key)
        return;
    auto dirUrl = parent;
    auto dirPath = parent.path();
    if (!dirPath.isEmpty() && dirPath != QDir::separator() && parent.path().endsWith(QDir::separator()))
        dirPath.chop(1);
    dirUrl.setPath(dirPath);
    filterAndSortFiles(dirUrl);
}

void FileSortWorker::handleFilters(QDir::Filters filters)
{
    resetFilters(filters);
}

void FileSortWorker::HandleNameFilters(const QStringList &filters)
{
    fmInfo() << "Handling name filters - count:" << filters.size();
    nameFilters = filters;
    QHash<QUrl, FileItemDataPointer>::iterator itr = childrenDataMap.begin();
    int processedCount = 0;
    for (; itr != childrenDataMap.end(); ++itr) {
        checkNameFilters(itr.value());
        processedCount++;
    }

    fmDebug() << "Name filters applied to" << processedCount << "items";
    Q_EMIT requestUpdateView();
}

void FileSortWorker::handleFilterData(const QVariant &data)
{
    if (isCanceled) {
        fmDebug() << "Ignoring filter data change - operation canceled";
        return;
    }

    fmInfo() << "Handling filter data change - valid data:" << data.isValid();

    filterData = data;
    if (!filterCallback || !data.isValid()) {
        fmDebug() << "No filter callback or invalid data - skipping filter operation";
        return;
    }

    filterAllFilesOrdered();
}

void FileSortWorker::handleFilterCallFunc(FileViewFilterCallback callback)
{
    if (isCanceled)
        return;

    filterCallback = callback;
    if (!filterCallback || !filterData.isValid())
        return;

    filterAllFilesOrdered();
}

void FileSortWorker::onToggleHiddenFiles()
{
    auto tmpfilters = filters;
    tmpfilters = ~(tmpfilters ^ QDir::Filter(~QDir::Hidden));
    resetFilters(tmpfilters);
}

void FileSortWorker::onShowHiddenFileChanged(bool isShow)
{
    if (isCanceled)
        return;
    QDir::Filters newFilters = filters;
    if (isShow) {
        newFilters |= QDir::Hidden;
    } else {
        newFilters &= ~QDir::Hidden;
    }

    handleFilters(newFilters);
}

void FileSortWorker::handleWatcherAddChildren(const QList<SortInfoPointer> &children)
{
    fmDebug() << "Handling watcher add children - count:" << children.size();

    for (const auto &sortInfo : children) {
        if (isCanceled) {
            fmDebug() << "Operation canceled during watcher add children";
            return;
        }

        if (this->children.value(makeParentUrl(sortInfo->fileUrl())).contains(sortInfo->fileUrl())) {
            auto data = childData(sortInfo->fileUrl());
            if (data && data->fileInfo())
                data->fileInfo()->updateAttributes();
            continue;
        }

        addChild(sortInfo, SortScenarios::kSortScenariosWatcherAddFile);
    }
}

void FileSortWorker::handleWatcherRemoveChildren(const QList<SortInfoPointer> &children)
{
    if (children.isEmpty()) {
        fmDebug() << "No children to remove from watcher";
        return;
    }

    fmDebug() << "Handling watcher remove children - count:" << children.size();

    auto parentUrl = makeParentUrl(children.first()->fileUrl());

    for (const auto &sortInfo : children) {
        if (isCanceled) {
            fmDebug() << "Operation canceled during watcher remove children";
            return;
        }

        if (sortInfo.isNull())
            continue;

        if (sortInfo->isDir() && visibleTreeChildren.keys().contains(sortInfo->fileUrl())) {
            fmDebug() << "Removing subdirectory:" << sortInfo->fileUrl().toString();
            removeSubDir(sortInfo->fileUrl());
            continue;
        }
    }

    auto subChildren = this->children.take(parentUrl);
    auto subVisibleList = visibleTreeChildren.take(parentUrl);
    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;

        if (sortInfo.isNull() || !subChildren.contains(sortInfo->fileUrl()))
            continue;

        subChildren.remove(sortInfo->fileUrl());
        subVisibleList.removeOne(sortInfo->fileUrl());

        {
            QWriteLocker lk(&childrenDataLocker);
            childrenDataMap.remove(sortInfo->fileUrl());
        }

        int showIndex = -1;
        {
            QReadLocker lk(&locker);
            if (!visibleChildren.contains(sortInfo->fileUrl()))
                continue;
            showIndex = visibleChildren.indexOf(sortInfo->fileUrl());
        }

        doModelChanged(ModelChangeType::kRemoveRows, showIndex, 1);
        {
            QWriteLocker lk(&locker);
            visibleChildren.removeAt(showIndex);
        }
        doModelChanged(ModelChangeType::kRemoveFinished);
    }

    this->children.insert(parentUrl, subChildren);
    visibleTreeChildren.insert(parentUrl, subVisibleList);
}

bool FileSortWorker::handleWatcherUpdateFile(const SortInfoPointer child)
{
    if (isCanceled)
        return false;

    if (!child)
        return false;

    if (!child->fileUrl().isValid() || !this->children.value(makeParentUrl(child->fileUrl())).contains(child->fileUrl()))
        return false;

    FileInfoPointer info;

    auto item = childData(child->fileUrl());
    if (item.isNull())
        return false;
    info = item->fileInfo();

    if (!info)
        return false;

    info->updateAttributes();

    sortInfoUpdateByFileInfo(info);

    return handleUpdateFile(child->fileUrl());
}

void FileSortWorker::handleWatcherUpdateFiles(const QList<SortInfoPointer> &children)
{
    for (auto sort : children) {
        if (isCanceled)
            return;
        handleWatcherUpdateFile(sort);
    }
}

void FileSortWorker::handleWatcherUpdateHideFile(const QUrl &hidUrl)
{
    if (isCanceled)
        return;
    auto hiddenFileInfo = InfoFactory::create<FileInfo>(hidUrl);
    if (!hiddenFileInfo)
        return;
    auto hidlist = DFMUtils::hideListFromUrl(QUrl::fromLocalFile(hiddenFileInfo->pathOf(PathInfoType::kFilePath)));
    auto parentUrl = makeParentUrl(hidUrl);
    for (const auto &child : children.value(parentUrl)) {
        if (isCanceled)
            return;

        auto item = childData(child->fileUrl());

        auto fileName = child->fileUrl().fileName();
        if (fileName.startsWith(".")) {
            child->setHide(true);
        } else {
            child->setHide(hidlist.contains(fileName));
        }
        auto info = item->fileInfo();
        if (!info)
            continue;
        info->setExtendedAttributes(ExtInfoType::kFileIsHid, child->isHide());
    }

    filterAndSortFiles(parentUrl, true, false);
}

void FileSortWorker::handleResort(const Qt::SortOrder order, const ItemRoles sortRole, const bool isMixDirAndFile)
{
    if (isCanceled) {
        fmDebug() << "Ignoring resort request - operation canceled";
        return;
    }

    fmInfo() << "Handling resort - order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
             << "role:" << static_cast<int>(sortRole) << "mix dir and file:" << isMixDirAndFile;

    auto opt = setSortArguments(order, sortRole, /*istree ? false :*/ isMixDirAndFile);

    switch (opt) {
    case FileSortWorker::SortOpt::kSortOptOtherChanged:
        fmDebug() << "Performing major resort - checking and updating file info";
        emit requestCursorWait();
        mimeSorting = this->sortRole == DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
        waitUpdatedFiles.clear();
        if (!checkAndUpdateFileInfoUpdate())
            return;
        return resortCurrent(false);
    case FileSortWorker::SortOpt::kSortOptOnlyOrderChanged:
        fmDebug() << "Performing simple reorder";
        emit requestCursorWait();
        return resortCurrent(true);
    default:
        fmDebug() << "No resort needed";
        return;
    }
}

void FileSortWorker::handleReGrouping(const Qt::SortOrder order, const QString &strategy, const QVariantHash &expansionStates)
{
    if (isCanceled) {
        fmDebug() << "Ignoring regrouping request - operation canceled";
        return;
    }

    auto opt = setGroupArguments(order, strategy, expansionStates);
    if (opt == GroupingOpt::kGroupingOptNone) {
        clearGroupedData();
        if (!currentIsGroupingMode()) {
            handleRefresh();
        }
        // 通知分组完成(实际上是取消分组)
        emit groupingFinished();
        return;
    }

    const QList<FileItemDataPointer> &allFiles = getAllFiles();
    if (allFiles.isEmpty()) {
        fmDebug() << "FileSortWorker: No files to group";
        clearGroupedData();
        emit groupingFinished();
        return;
    }

    if (opt == GroupingOpt::kGroupingOptOnlyOrderChanged) {
        // Set group order in engine (support for ascending/descending order)
        groupingEngine->setGroupOrder(groupOrder);
        groupingEngine->reorderGroups(&groupedModelData);
        emit groupDataChanged();
    } else {
        applyGrouping(allFiles);
    }

    fmInfo() << "FileSortWorker: Grouping completed - created" << groupedModelData.groups.size()
             << "groups with" << groupedModelData.getItemCount() << "total items";

    // 通知分组完成
    emit groupingFinished();
}

void FileSortWorker::handleGroupingChanged()
{
    // Early return for invalid state
    if (!isCurrentGroupingEnabled || isCanceled
        || (groupedModelData.isEmpty() && visibleChildren.isEmpty())) {
        fmDebug() << "Ignoring grouping update - no grouping data";
        return;
    }

    if (!groupedModelData.isEmpty() && visibleChildren.isEmpty()) {
        // Remove all files - clear grouping
        fmDebug() << "clearing grouping";
        clearGroupedData();
        return;
    }

    // transform insert to update if all files are visible
    if (groupingEngine->currentUpdateMode() == GroupingEngine::UpdateMode::kInsert) {
        if (!visibleChildren.isEmpty() && groupedModelData.getFileItemCount() == visibleChildren.count()) {
            groupingEngine->setUpdateMode(GroupingEngine::UpdateMode::kUpdate);
        }
    }

    // Handle different update modes
    switch (groupingEngine->currentUpdateMode()) {
    case GroupingEngine::UpdateMode::kInsert: {
        handleGroupingInsert();
        break;
    }
    case GroupingEngine::UpdateMode::kRemove: {
        handleGroupingRemove();
        break;
    }
    case GroupingEngine::UpdateMode::kUpdate: {
        handleGroupingUpdate();
        break;
    }
    case GroupingEngine::UpdateMode::kNoGrouping:
    default:
        // Do nothing for unsupported modes
        break;
    }
}

void FileSortWorker::handleGroupingInsert()
{
    auto range = groupingEngine->currentUpdateChildrenRange();
    bool isCoverage = range.first == 0 && range.second == visibleChildren.count();

    // 插入数据的范围是整个历史数据，说明发生了覆盖，直接重新分组
    if (isCoverage) {
        fmDebug() << "applying grouping";
        applyGrouping(getAllFiles());
        return;
    }

    // 截取新插入的数据
    auto anchor = groupingEngine->findPrecedingAnchor(visibleChildren, range);
    if (!anchor.has_value()) {
        fmWarning() << "Failed to find preceding anchor";
        return;
    }

    groupingEngine->setUpdateChildren(visibleChildren.mid(range.first, range.second));

    QReadLocker datalocker(&childrenDataLocker);
    QReadLocker visibleChildrenLocker(&locker);
    const auto &result = groupingEngine->insertFilesToModelData(anchor.value(),
                                                                groupedModelData, currentStrategy);
    datalocker.unlock();
    visibleChildrenLocker.unlock();
    if (!result.success) {
        fmWarning() << "Failed to insert file to grouping data";
        if (result.alwaysUpdate) {
            groupedModelData = result.newData;
            emit groupDataChanged();
        }
        return;
    }

    doModelChanged(ModelChangeType::kInsertGroupRows, result.pos, result.count);
    groupedModelData = result.newData;
    doModelChanged(ModelChangeType::kInsertGroupFinished);
}

void FileSortWorker::handleGroupingRemove()
{
    QReadLocker datalocker(&childrenDataLocker);
    const auto &result = groupingEngine->removeFilesFromModelData(groupedModelData);
    datalocker.unlock();

    if (!result.success) {
        fmWarning() << "Failed to remove file from grouping data";
        // TODO: perf
        applyGrouping(getAllFiles());
        return;
    }
    doModelChanged(ModelChangeType::kRemoveGroupRows, result.pos, result.count);
    groupedModelData = result.newData;
    doModelChanged(ModelChangeType::kRemoveGroupFinished);
}

void FileSortWorker::handleGroupingUpdate()
{
    QReadLocker datalocker(&childrenDataLocker);
    QReadLocker visibleChildrenLocker(&locker);
    auto range = groupingEngine->currentUpdateChildrenRange();
    datalocker.unlock();
    visibleChildrenLocker.unlock();

    if (range.first == 0 && range.second == visibleChildren.count()) {
        fmDebug() << "applying grouping";
        applyGrouping(getAllFiles());
        return;
    }

    auto anchor = groupingEngine->findPrecedingAnchor(visibleChildren, range);
    if (!anchor.has_value()) {
        fmWarning() << "Failed to find preceding anchor";
        return;
    }

    groupingEngine->setUpdateChildren(visibleChildren.mid(range.first, range.second));
    const auto &result = groupingEngine->updateFilesToModelData(anchor.value(),
                                                                groupedModelData, currentStrategy);

    if (!result.success) {
        fmWarning() << "Failed to update file to grouping data";
        return;
    }

    doModelChanged(ModelChangeType::kInsertGroupRows, result.pos, result.count);
    groupedModelData = result.newData;
    doModelChanged(ModelChangeType::kInsertGroupFinished);
}

void FileSortWorker::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (isCanceled || istree)
        return;

    if (aa == Application::kFileAndDirMixedSort) {
        handleResort(sortOrder, orgSortRole, value.toBool());
    }
}

bool FileSortWorker::handleUpdateFile(const QUrl &url)
{
    if (isCanceled)
        return false;

    if (!url.isValid())
        return false;

    SortInfoPointer sortInfo = children.value(makeParentUrl(url)).value(url);
    if (!sortInfo)
        return false;

    bool childVisible = false;
    int childIndex = -1;
    {
        QReadLocker lk(&locker);
        childVisible = visibleChildren.contains(url);
        childIndex = visibleChildren.indexOf(url);
    }

    if (childVisible) {
        if (!checkFilters(sortInfo, true)) {
            doModelChanged(ModelChangeType::kRemoveRows, childIndex, 1);
            {
                QWriteLocker lk(&locker);
                visibleChildren.removeAt(childIndex);
            }
            doModelChanged(ModelChangeType::kRemoveFinished);
            return false;
        }
        Q_EMIT updateRow(childIndex);
        return false;
    }

    bool added = false;
    if (checkFilters(sortInfo, true)) {
        auto parentUrl = makeParentUrl(sortInfo->fileUrl());
        int showIndex = findStartPos(parentUrl);

        // 插入到每个目录下的显示目录
        auto subVisibleList = visibleTreeChildren.take(parentUrl);
        auto offset = subVisibleList.count();
        if (orgSortRole != Global::ItemRoles::kItemDisplayRole)
            offset = insertSortList(sortInfo->fileUrl(), subVisibleList, SortScenarios::kSortScenariosWatcherAddFile);
        auto subIndex = offset;
        // 根目录下的offset计算不一样
        if (UniversalUtils::urlEquals(parentUrl, current)) {
            if (offset >= subVisibleList.count() || offset == 0) {
                offset = offset >= subVisibleList.count() ? childrenCountInternal() : 0;
            } else {
                auto tmpUrl = offset >= subVisibleList.length() ? QUrl() : subVisibleList.at(offset);
                offset = getChildShowIndexInternal(tmpUrl);
                if (offset < 0)
                    offset = childrenCountInternal();
            }
        }

        insertToList(subVisibleList, subIndex, sortInfo->fileUrl());

        visibleTreeChildren.insert(parentUrl, subVisibleList);

        // kItemDisplayRole 是不进行排序的
        showIndex += offset;

        // 不为子目录中第一项的情况下，需要判断前面的项是否有展开
        if (subIndex != 0) {
            QUrl preItemUrl = subVisibleList.at(subIndex - 1);
            // 前一项展开的情况下，实际插入的位置应该在所有展开子项之后
            showIndex = findRealShowIndex(preItemUrl);
        }

        if (isCanceled)
            return false;

        doModelChanged(ModelChangeType::kInsertRows, showIndex, 1);
        {
            QWriteLocker lk(&locker);
            insertToList(visibleChildren, showIndex, sortInfo->fileUrl());
        }
        doModelChanged(ModelChangeType::kInsertFinished);
        added = true;

        // async create file will add to view while file info updated.
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());
    }

    return added;
}

void FileSortWorker::handleUpdateFiles(const QList<QUrl> &urls)
{
    for (auto const &url : urls) {
        if (isCanceled)
            return;
        handleUpdateFile(url);
    }
}

void FileSortWorker::handleRefresh()
{
    fmInfo() << "Handling refresh operation";

    int childrenCount = this->childrenCountInternal();
    if (childrenCount > 0)
        doModelChanged(ModelChangeType::kRemoveRows, 0, childrenCount);

    {
        QWriteLocker lk(&locker);
        visibleChildren.clear();
    }
    children.clear();
    visibleTreeChildren.clear();
    depthMap.clear();
    if (isCurrentGroupingEnabled) {
        clearGroupedData();
    }

    {
        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.clear();
    }

    if (childrenCount > 0)
        doModelChanged(ModelChangeType::kRemoveFinished);

    Q_EMIT requestFetchMore();
}

void FileSortWorker::handleClearThumbnail()
{
    QReadLocker lk(&childrenDataLocker);
    for (const auto &item : childrenDataMap.values()) {
        if (Q_LIKELY(item))
            item->clearThumbnail();
    }

    Q_EMIT requestUpdateView();
}

void FileSortWorker::handleFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg)
{
    Q_UNUSED(isLinkOrg);
    if (!children.value(makeParentUrl(url)).contains(url))
        return;

    auto itemdata = childData(url);
    if (!itemdata)
        return;

    auto fileInfo = itemdata->fileInfo();
    if (!fileInfo || QString::number(quintptr(fileInfo.data()), 16) != infoPtr)
        return;

    itemdata->transFileInfo();

    fileInfo = itemdata->fileInfo();
    fileInfo->customData(Global::ItemRoles::kItemFileRefreshIcon);
    checkAndSortBytMimeType(fileInfo->fileUrl());
    sortInfoUpdateByFileInfo(fileInfo);

    if (fileInfoRefresh.contains(url))
        return;

    fileInfoRefresh.append(url);

    if (updateRefresh && updateRefresh->isActive())
        return;

    if (!updateRefresh) {
        updateRefresh = new QTimer(this);   // 设置parent，确保在对象销毁时自动清理
        connect(updateRefresh, &QTimer::timeout, this, &FileSortWorker::handleUpdateRefreshFiles, Qt::QueuedConnection);
    }
    updateRefresh->setSingleShot(true);
    updateRefresh->setInterval(200);
    updateRefresh->start();
}

void FileSortWorker::handleUpdateRefreshFiles()
{
    if (fileInfoRefresh.isEmpty())
        return;
    handleUpdateFiles(fileInfoRefresh);
    fileInfoRefresh.clear();
}

void FileSortWorker::handleSortByMimeType()
{
    if (isCanceled)
        return;
    resortCurrent(false);
}

void FileSortWorker::handleAboutToInsertFilesToGroup(int pos, int count)
{
    if (!isCurrentGroupingEnabled || isCanceled) {
        return;
    }

    groupingEngine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    // 对于数据插入，此时还没有更新完成，无法获取children,
    // 只能先记录范围，后续在handleGroupingUpdate完成更新
    groupingEngine->setUpdateChildrenRange(pos, count);
}

void FileSortWorker::handleAboutToRemoveFilesFromGroup(int pos, int count)
{
    if (!isCurrentGroupingEnabled || isCanceled) {
        return;
    }

    groupingEngine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    groupingEngine->setUpdateChildren(visibleChildren.mid(pos, count));
    // 对于移除，设置范围没意义，所以这里只是重置
    groupingEngine->setUpdateChildrenRange(0, 0);
}

void FileSortWorker::handleToggleGroupExpansion(const QString &key, const QString &groupKey)
{
    if (groupKey.isEmpty() || !isCurrentGroupingEnabled || isCanceled || key != currentKey) {
        return;
    }
    auto group = groupedModelData.getGroup(groupKey);
    if (!group) {
        fmWarning() << "FileSortWorker: Group" << groupKey << "not found";
        return;
    }
    int count = group->fileCount;
    int pos = groupedModelData.findGroupHeaderStartPos(groupKey).value_or(-1);

    if (pos < 0) {
        fmWarning() << "FileSortWorker: Group" << groupKey << "not found";
        return;
    }

    bool currentState = groupExpansionStates.value(groupKey, true);
    bool newState = !currentState;
    groupExpansionStates[groupKey] = newState;

    if (currentState) {
        // current is expanded
        doModelChanged(ModelChangeType::kRemoveGroupRows, pos + 1, count);
    } else {
        doModelChanged(ModelChangeType::kInsertGroupRows, pos + 1, count);
    }

    groupedModelData.setGroupExpanded(groupKey, newState);

    if (currentState) {
        // current is expanded
        doModelChanged(ModelChangeType::kRemoveGroupFinished);
    } else {
        doModelChanged(ModelChangeType::kInsertGroupFinished);
    }

    Q_EMIT groupExpansionChanged(currentStrategy->getStrategyName(), groupKey, newState);

    fmDebug() << "FileSortWorker: Group" << groupKey << (currentState ? "collapsed" : "expanded");
}

void FileSortWorker::handleCloseExpand(const QString &key, const QUrl &parent)
{
    if (isCanceled || key != currentKey || UniversalUtils::urlEquals(parent, current))
        return;
    if (!children.keys().contains(parent))
        return;
    removeSubDir(parent);
}

void FileSortWorker::handleSwitchTreeView(const bool isTree)
{
    if (isTree == istree) {
        fmDebug() << "Tree view mode unchanged - current:" << istree;
        return;
    }

    fmInfo() << "Switching view mode from" << (istree ? "tree" : "list") << "to" << (isTree ? "tree" : "list");

    istree = isTree;
    if (istree) {
        fmDebug() << "Switching to tree view mode";
        switchTreeView();
    } else {
        fmDebug() << "Switching to list view";
        switchListView();
    }
}

void FileSortWorker::handleAddChildren(const QString &key,
                                       QList<SortInfoPointer> children,
                                       const QList<FileInfoPointer> &childInfos,
                                       const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                                       const Qt::SortOrder sortOrder,
                                       const bool isMixDirAndFile,
                                       const bool handleSource,
                                       const bool isFinished,
                                       const bool isSort,
                                       const bool isFirstBatch)
{
    if (!handleAddChildren(key, children, childInfos, isFirstBatch))
        return;

    if (children.isEmpty()) {
        if (handleSource)
            setSourceHandleState(isFinished);

        return;
    }

    // In the home, it is necessary to sort by display name.
    // So, using `sortAllFiles` to reorder
    const auto parentUrl = makeParentUrl(children.first()->fileUrl());
    const auto path = FileUtils::bindPathTransform(parentUrl.path(), false);
    bool isHome = (path == StandardPaths::location(StandardPaths::kHomePath));
    if (!isHome && sortRole != DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault && this->sortRole == sortRole
        && this->sortOrder == sortOrder && this->isMixDirAndFile == isMixDirAndFile) {
        if (handleSource)
            setSourceHandleState(isFinished);
        return;
    }

    if (isCanceled)
        return;
    // 对当前的目录排序， 若果处理的是获取源数据，在没有获取完，不进行排序
    if ((!handleSource || isFinished) && isSort) {
        auto startPos = findStartPos(parentUrl);
        auto sortList = sortTreeFiles(visibleTreeChildren.take(parentUrl));
        // 找到endpos
        insertVisibleChildren(startPos, sortList, InsertOpt::kInsertOptReplace, startPos + sortList.length());
    }

    if (handleSource)
        setSourceHandleState(isFinished);
}

bool FileSortWorker::handleAddChildren(const QString &key,
                                       const QList<SortInfoPointer> &children,
                                       const QList<FileInfoPointer> &childInfos,
                                       const bool isFirstBatch)
{
    if (currentKey != key || isCanceled)
        return false;
    if (children.isEmpty())
        return true;

    // Clear old data when receiving first batch of items in kPreserve mode
    if (isFirstBatch && !istree) {
        visibleTreeChildren.clear();
        // Clear the existing children data when we're about to insert the first batch
        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.clear();

        QWriteLocker vlk(&locker);
        visibleChildren.clear();

        this->children.clear();
    }

    // 获取相对于已有的新增加的文件
    QList<QUrl> newChildren;

    auto parentUrl = makeParentUrl(children.first()->fileUrl());
    // 获取当前的插入的位置
    auto childUrls = visibleTreeChildren.take(parentUrl);
    auto startPos = findStartPos(parentUrl);
    auto posOffset = childUrls.length();
    QHash<QUrl, SortInfoPointer> tmpChildren = this->children.take(parentUrl);
    // 辅助或者fileinfo
    int index = 0;
    int infosSize = childInfos.count();
    // 获取深度
    auto depth = findDepth(parentUrl);
    for (const auto &sortInfo : children) {
        if (tmpChildren.contains(sortInfo->fileUrl()))
            continue;
        tmpChildren.insert(sortInfo->fileUrl(), sortInfo);
        if (checkFilters(sortInfo))
            newChildren.append(sortInfo->fileUrl());
        if (isCanceled)
            return false;
        FileInfoPointer info { nullptr };
        if (infosSize > 0 && index < infosSize)
            info = childInfos.at(index);
        createAndInsertItemData(depth, sortInfo, info);
        index++;
    }

    this->children.insert(parentUrl, tmpChildren);
    childUrls.append(newChildren);
    visibleTreeChildren.insert(parentUrl, childUrls);
    depthMap.remove(depth - 1, parentUrl);
    depthMap.insert(depth - 1, parentUrl);
    if (newChildren.isEmpty())
        return true;

    insertVisibleChildren(startPos + posOffset, newChildren);

    return true;
}

void FileSortWorker::setSourceHandleState(const bool isFinished)
{
    if (isFinished) {
        Q_EMIT requestSetIdel(visibleChildren.count(), childrenDataMap.count());
    } else {
        Q_EMIT getSourceData(currentKey);
    }
}

void FileSortWorker::resetFilters(const QDir::Filters filters)
{
    if (isCanceled)
        return;
    if (this->filters == filters)
        return;

    this->filters = filters;
    filterAllFilesOrdered();
}

void FileSortWorker::checkNameFilters(const FileItemDataPointer itemData)
{
    if (!itemData || itemData->data(Global::ItemRoles::kItemFileIsDirRole).toBool() || nameFilters.isEmpty())
        return;

    QRegularExpression re("", QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < nameFilters.size(); ++i) {
        QString pattern = QRegularExpression::wildcardToRegularExpression(nameFilters.at(i));
        re.setPattern(pattern);
        if (re.match(itemData->data(kItemNameRole).toString()).hasMatch()) {
            itemData->setAvailableState(true);
            return;
        }
    }

    itemData->setAvailableState(false);
}

void FileSortWorker::filterAllFilesOrdered()
{
    visibleTreeChildren.clear();
    filterAndSortFiles(current, true, false);
}

void FileSortWorker::filterAndSortFiles(const QUrl &dir, const bool fileter, const bool reverse)
{
    if (isCanceled)
        return;
    // 先排深度是0的url
    QList<QUrl> visibleList;
    auto startPos = findStartPos(dir);
    int endPos = -1;
    // 找到父目录了
    if (!(fileter || UniversalUtils::urlEquals(dir, current) || reverse)) {
        endPos = findEndPos(dir);
    }
    // 执行过滤
    QList<QUrl> removeDirs;
    if (fileter)
        removeDirs = filterFilesByParent(dir, true);

    // 执行排序
    if (istree)
        visibleList = sortAllTreeFilesByParent(dir, reverse);
    else {
        visibleList = sortTreeFiles(visibleTreeChildren.contains(current) ? visibleTreeChildren[current] : visibleChildren, reverse);
    }

    // 执行界面刷新  设置过滤，当前的目录是当前树的根目录，反序。所有的显示url都要改变
    if (fileter || UniversalUtils::urlEquals(dir, current) || reverse) {
        insertVisibleChildren(startPos, visibleList, InsertOpt::kInsertOptForce);
    } else {
        insertVisibleChildren(startPos, visibleList, InsertOpt::kInsertOptReplace, endPos);
    }

    // 移除所有的children，和itemdata
    if (!removeDirs.isEmpty()) {
        auto removeItemList = removeChildrenByParents(removeDirs);
        if (!removeItemList.isEmpty())
            removeFileItems(removeItemList);
    }
}

void FileSortWorker::resortCurrent(const bool reverse)
{
    if (isCanceled)
        return;

    QList<QUrl> visibleList;

    // 执行排序
    if (istree)
        visibleList = sortAllTreeFilesByParent(current, reverse);
    else {
        visibleList = sortTreeFiles(visibleTreeChildren.contains(current) ? visibleTreeChildren[current] : visibleChildren, reverse);
    }

    resortVisibleChildren(visibleList);
    if (isCurrentGroupingEnabled) {
        applyGrouping(getAllFiles());
    }
    emit reqUestCloseCursor();
}

QList<QUrl> FileSortWorker::filterFilesByParent(const QUrl &dir, const bool byInfo)
{
    // 先排深度是0的url
    int8_t depth = getDepth(dir);
    depth = depth < 0 ? -1 : depth;

    QList<QUrl> allSubUnShowDir;
    QList<QUrl> depthParentUrls { dir };
    // 执行
    while (!depthParentUrls.isEmpty()) {
        if (isCanceled)
            return {};
        for (const auto &parent : depthParentUrls) {
            if (isCanceled)
                return {};
            if (!UniversalUtils::urlEquals(parent, current) && !UniversalUtils::isParentUrl(parent, dir))
                continue;
            auto sortInfo = children.value(makeParentUrl(parent)).value(parent);
            if (sortInfo && sortInfo->needsCompletion())
                doCompleteFileInfo(sortInfo);
            if (!UniversalUtils::urlEquals(parent, current) && !checkFilters(sortInfo, byInfo)) {
                allSubUnShowDir.append(removeVisibleTreeChildren(parent));
                continue;
            }

            filterTreeDirFiles(parent, byInfo);
        }

        depthParentUrls = depthMap.values(++depth);
    }

    return allSubUnShowDir;
}

void FileSortWorker::filterTreeDirFiles(const QUrl &parent, const bool byInfo)
{
    if (isCanceled)
        return;

    QList<QUrl> filterUrls {};
    for (const auto &sortInfo : children.value(parent)) {
        if (isCanceled)
            return;

        if (sortInfo && sortInfo->needsCompletion())
            doCompleteFileInfo(sortInfo);

        if (checkFilters(sortInfo, byInfo))
            filterUrls.append(sortInfo->fileUrl());
    }

    visibleTreeChildren.remove(parent);
    if (filterUrls.isEmpty()) {
        if (UniversalUtils::urlEquals(parent, current)) {
            doModelChanged(ModelChangeType::kRemoveRows, 0, visibleChildren.count());
            {
                QWriteLocker lk(&locker);
                visibleChildren.clear();
            }
            doModelChanged(ModelChangeType::kRemoveFinished);
        }
        return;
    }

    visibleTreeChildren.insert(parent, filterUrls);
}

bool FileSortWorker::addChild(const SortInfoPointer &sortInfo,
                              const SortScenarios sort)
{
    if (isCanceled || sortInfo.isNull())
        return false;

    auto parentUrl = makeParentUrl(sortInfo->fileUrl());
    auto depth = findDepth(parentUrl);
    if (depth < 0)
        return false;

    if (children.value(parentUrl).contains(sortInfo->fileUrl()))
        return false;

    auto childList = children.take(parentUrl);
    childList.insert(sortInfo->fileUrl(), sortInfo);
    children.insert(parentUrl, childList);
    {
        auto info = InfoFactory::create<FileInfo>(sortInfo->fileUrl());
        if (info)
            info->updateAttributes();
        createAndInsertItemData(depth, sortInfo, info);
    }

    depthMap.remove(depth - 1, parentUrl);
    depthMap.insert(depth - 1, parentUrl);

    if (!checkFilters(sortInfo, true))
        return false;

    if (isCanceled)
        return false;

    int showIndex = findStartPos(parentUrl);

    // 插入到每个目录下的显示目录
    auto subVisibleList = visibleTreeChildren.take(parentUrl);
    auto offset = subVisibleList.length();
    if (orgSortRole != Global::ItemRoles::kItemDisplayRole)
        offset = insertSortList(sortInfo->fileUrl(), subVisibleList, sort);
    auto subIndex = offset;
    // 根目录下的offset计算不一样
    if (UniversalUtils::urlEquals(parentUrl, current)) {
        if (offset >= subVisibleList.length() || offset == 0) {
            offset = offset >= subVisibleList.length() ? childrenCountInternal() : 0;
        } else {
            auto tmpUrl = offset >= subVisibleList.length() ? QUrl() : subVisibleList.at(offset);
            offset = getChildShowIndexInternal(tmpUrl);
            if (offset < 0)
                offset = childrenCountInternal();
        }
    }

    insertToList(subVisibleList, subIndex, sortInfo->fileUrl());
    visibleTreeChildren.insert(parentUrl, subVisibleList);

    // kItemDisplayRole 是不进行排序的
    showIndex += offset;

    // 不为子目录中第一项的情况下，需要判断前面的项是否有展开
    if (subIndex != 0) {
        QUrl preItemUrl = subVisibleList.at(subIndex - 1);
        // 前一项展开的情况下，实际插入的位置应该在所有展开子项之后
        showIndex = findRealShowIndex(preItemUrl);
    }

    if (isCanceled)
        return false;

    doModelChanged(ModelChangeType::kInsertRows, showIndex, 1);
    {
        QWriteLocker lk(&locker);
        insertToList(visibleChildren, showIndex, sortInfo->fileUrl());
    }
    doModelChanged(ModelChangeType::kInsertFinished);

    if (sort == SortScenarios::kSortScenariosWatcherAddFile)
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());

    return true;
}

bool FileSortWorker::sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo)
{
    if (!fileInfo)
        return false;

    auto url = fileInfo->fileUrl();
    auto parentUrl = makeParentUrl(url);
    if (!children.value(parentUrl).contains(url))
        return false;

    SortInfoPointer sortInfo = children.value(parentUrl).value(url);
    if (!sortInfo)
        return false;

    sortInfo->setUrl(fileInfo->urlOf(UrlInfoType::kUrl));
    sortInfo->setSize(fileInfo->size());
    sortInfo->setFile(fileInfo->isAttributes(OptInfoType::kIsFile));
    sortInfo->setDir(fileInfo->isAttributes(OptInfoType::kIsDir));
    sortInfo->setHide(fileInfo->isAttributes(OptInfoType::kIsHidden));
    sortInfo->setSymlink(fileInfo->isAttributes(OptInfoType::kIsSymLink));
    sortInfo->setReadable(fileInfo->isAttributes(OptInfoType::kIsReadable));
    sortInfo->setWriteable(fileInfo->isAttributes(OptInfoType::kIsWritable));
    sortInfo->setExecutable(fileInfo->isAttributes(OptInfoType::kIsExecutable));
    sortInfo->setLastReadTime(fileInfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setLastModifiedTime(fileInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setCreateTime(fileInfo->timeOf(TimeInfoType::kCreateTime).value<QDateTime>().toSecsSinceEpoch());
    fileInfo->fileMimeType();

    return true;
}

void FileSortWorker::switchTreeView()
{
    // 当前只有一层，只需要展开获取每个目录的展开属性,只有父母这一层
    if (isMixDirAndFile) {
        fmDebug() << "Disabling mixed dir and file sorting for tree view";
        handleResort(sortOrder, orgSortRole, false);
    }
    emit requestUpdateView();
}

void FileSortWorker::switchListView()
{
    // 移除depthMap和visibleTreeChildren
    auto allShowList = visibleTreeChildren.value(current);
    // 保持选中
    Q_EMIT aboutToSwitchToListView(allShowList);

    visibleTreeChildren.clear();
    depthMap.clear();
    depthMap.insert(-1, current);
    auto oldMix = isMixDirAndFile;
    isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    // 排序
    if (isMixDirAndFile != oldMix) {
        allShowList = sortTreeFiles(allShowList);
    } else {
        visibleTreeChildren.insert(current, allShowList);
    }

    // 更新显示项
    insertVisibleChildren(0, allShowList, InsertOpt::kInsertOptForce);
    // 移除children
    auto allShowChildren = children.value(current);
    QList<QUrl> removeChildren;
    for (const auto &url : children.keys()) {
        if (UniversalUtils::urlEquals(url, current))
            continue;
        removeChildren.append(children.value(url).keys());
    }
    children.clear();
    children.insert(current, allShowChildren);
    // 移除fileitem
    QWriteLocker lk(&childrenDataLocker);
    for (const auto &url : removeChildren)
        childrenDataMap.remove(url);

    for (auto itemData : childrenDataMap)
        itemData->setExpanded(false);
}

QList<QUrl> FileSortWorker::sortAllTreeFilesByParent(const QUrl &dir, const bool reverse)
{
    QList<QUrl> visibleList;
    int8_t depth = getDepth(dir);
    if (depth <= -2)
        return {};
    QList<QUrl> depthParentUrls { dir };
    bool bSort = orgSortRole != Global::ItemRoles::kItemDisplayRole;
    // 执行排序
    while (!depthParentUrls.isEmpty()) {
        if (isCanceled)
            return {};
        for (const auto &parent : depthParentUrls) {
            if (isCanceled)
                return {};
            if (!UniversalUtils::urlEquals(dir, parent) && !UniversalUtils::isParentUrl(parent, dir))
                continue;

            QList<QUrl> sortList {};
            if (visibleTreeChildren.isEmpty() && UniversalUtils::urlEquals(parent, current)) {
                sortList = sortTreeFiles(visibleChildren, reverse);
            } else {
                sortList = bSort ? sortTreeFiles(visibleTreeChildren.take(parent), reverse) : visibleTreeChildren.value(parent);
            }

            if (sortList.isEmpty())
                continue;
            auto startPos = findStartPos(visibleList, parent);
            QList<QUrl> tmp;
            tmp.append(visibleList.mid(0, startPos));
            tmp.append(sortList);
            tmp.append(visibleList.mid(startPos));
            visibleList = tmp;
        }
        // 获取下一级的depthParentUrls
        depthParentUrls = depthMap.values(++depth);
    }

    return visibleList;
}

QList<QUrl> FileSortWorker::sortTreeFiles(const QList<QUrl> &children, const bool reverse)
{
    if (isCanceled || children.isEmpty())
        return {};

    auto parentUrl = makeParentUrl(children.first());
    if (orgSortRole == Global::ItemRoles::kItemDisplayRole) {
        visibleTreeChildren.insert(parentUrl, children);
        return {};
    }

    if (children.count() <= 1) {
        visibleTreeChildren.insert(parentUrl, children);
        return children;
    }

    QList<QUrl> sortList;
    int sortIndex = 0;
    QHash<QUrl, SortInfoPointer> sortInfos = reverse && !isMixDirAndFile ? this->children.value(parentUrl)
                                                                         : QHash<QUrl, SortInfoPointer>();
    bool firstFile = false;
    for (const auto &url : children) {
        if (isCanceled)
            return {};
        if (!reverse) {
            sortIndex = insertSortList(url, sortList, SortScenarios::kSortScenariosNormal);
        } else if (!firstFile && !isMixDirAndFile) {
            auto sortInfo = sortInfos.value(url);
            if (sortInfo && sortInfo->needsCompletion())
                doCompleteFileInfo(sortInfo);
            if (sortInfo && sortInfo->isFile()) {
                firstFile = true;
                sortIndex = sortList.count();
            }
        }
        insertToList(sortList, sortIndex, url);
    }

    if (sortList.isEmpty())
        return {};

    visibleTreeChildren.insert(parentUrl, sortList);

    return sortList;
}

QList<QUrl> FileSortWorker::removeChildrenByParents(const QList<QUrl> &dirs)
{
    QList<QUrl> urls;
    for (const auto &dir : dirs) {
        for (const auto &sortInfo : children.value(dir))
            urls << sortInfo->fileUrl();
        children.remove(dir);
        auto item = childData(dir);
        if (item)
            item->setExpanded(false);
    }
    return urls;
}

QList<QUrl> FileSortWorker::removeVisibleTreeChildren(const QUrl &parent)
{
    auto depth = depthMap.key(parent);
    QList<QUrl> depthParentUrls = depthMap.values(depth);
    QList<QUrl> removeUrls {};
    while (!depthParentUrls.isEmpty()) {
        if (isCanceled)
            return {};
        for (const auto &child : depthParentUrls) {
            if (UniversalUtils::urlEquals(child, parent) || UniversalUtils::isParentUrl(child, parent)) {
                if (!removeUrls.contains(child))
                    removeUrls.append(child);
                visibleTreeChildren.remove(child);
                depthMap.remove(depth, child);
            }
        }

        // 获取下一级的depthParentUrls
        depthParentUrls = depthMap.values(++depth);
    }
    return removeUrls;
}

void FileSortWorker::removeSubDir(const QUrl &dir)
{
    auto startPos = findStartPos(dir);
    auto endPos = findEndPos(dir);

    // 移除可显示的所有的url
    auto removeDir = removeVisibleTreeChildren(dir);
    // 移除界面所有显示的url
    removeVisibleChildren(startPos, endPos == -1 ? childrenCountInternal() - startPos : endPos - startPos);
    // 移除itemdata
    if (removeDir.isEmpty())
        return;
    auto removeList = removeChildrenByParents(removeDir);
    if (removeList.isEmpty())
        return;
    removeFileItems(removeList);
}

void FileSortWorker::removeFileItems(const QList<QUrl> &urls)
{
    QWriteLocker lk(&childrenDataLocker);
    for (const auto &url : urls)
        childrenDataMap.remove(url);
}

int8_t FileSortWorker::findDepth(const QUrl &parent)
{
    if (UniversalUtils::urlEquals(parent, current))
        return 0;
    auto parentItem = childData(parent);
    if (parentItem.isNull())
        return -1;
    return parentItem->data(Global::ItemRoles::kItemTreeViewDepthRole).value<int8_t>() + 1;
}

int FileSortWorker::findEndPos(const QUrl &dir)
{
    if (UniversalUtils::urlEquals(dir, current))
        return childrenCountInternal();

    const auto &parentUrl = makeParentUrl(dir);
    auto index = visibleTreeChildren.value(parentUrl).indexOf(dir);
    if (index < 0)
        return -1;

    if (index == visibleTreeChildren.value(parentUrl).length() - 1)
        return findEndPos(makeParentUrl(dir));

    return getChildShowIndexInternal(visibleTreeChildren.value(parentUrl).at(index + 1));
}

int FileSortWorker::findStartPos(const QUrl &parent)
{
    if (UniversalUtils::urlEquals(parent, current))
        return 0;
    auto pos = getChildShowIndexInternal(parent);
    // 在父目录的后面一个位置插入
    return pos < 0 ? pos : pos + 1;
}

int FileSortWorker::findStartPos(const QList<QUrl> &list, const QUrl &parent)
{
    if (UniversalUtils::urlEquals(parent, current))
        return 0;
    auto pos = list.indexOf(parent);
    // 在父目录的后面一个位置插入
    return pos < 0 ? pos : pos + 1;
}

void FileSortWorker::resortVisibleChildren(const QList<QUrl> &fileUrls)
{
    if (isCanceled)
        return;

    int count = setVisibleChildren(0, fileUrls, InsertOpt::kInsertOptForce, -1);
    if (count > 0)
        Q_EMIT dataChanged(0, count - 1);
}

void FileSortWorker::insertVisibleChildren(const int startPos, const QList<QUrl> &filterUrls,
                                           const InsertOpt opt, const int endPos)
{
    if (isCanceled)
        return;

    doModelChanged(ModelChangeType::kInsertRows, startPos, filterUrls.length());
    setVisibleChildren(startPos, filterUrls, opt, endPos);
    doModelChanged(ModelChangeType::kInsertFinished);
}

void FileSortWorker::removeVisibleChildren(const int startPos, const int size)
{
    if (isCanceled || size <= 0)
        return;
    doModelChanged(ModelChangeType::kRemoveRows, startPos, size);
    {
        auto tmp = getChildrenUrls();
        QList<QUrl> visibleList;
        visibleList.append(tmp.mid(0, startPos));
        visibleList.append(tmp.mid(startPos + size));
        if (isCanceled)
            return;

        QWriteLocker lk(&locker);
        visibleChildren = visibleList;
    }

    doModelChanged(ModelChangeType::kRemoveFinished, 0, 0);
}

void FileSortWorker::createAndInsertItemData(const int8_t depth, const SortInfoPointer child, const FileInfoPointer info)
{
    // 设置
    FileItemDataPointer item { nullptr };
    if (info.isNull()) {
        item.reset(new FileItemData(child, rootdata.data()));
    } else {
        item.reset(new FileItemData(child->fileUrl(), info, rootdata.data()));
        item->setSortFileInfo(child);
    }

    item->setDepth(depth);

    QWriteLocker lk(&childrenDataLocker);
    childrenDataMap.insert(child->fileUrl(), item);
}

int FileSortWorker::insertSortList(const QUrl &needNode, const QList<QUrl> &list,
                                   SortScenarios sort)
{
    int begin = 0;
    int end = list.count();

    if (end <= 0)
        return 0;

    if (isCanceled)
        return 0;

    if ((sortOrder == Qt::AscendingOrder) ^ !lessThan(needNode, list.first(), sort))
        return 0;

    if ((sortOrder == Qt::AscendingOrder) ^ lessThan(needNode, list.last(), sort))
        return list.count();

    int row = (begin + end) / 2;

    // 先找到文件还是目录
    forever {
        if (isCanceled)
            return row;

        if (begin == end)
            break;

        const QUrl &node = list.at(row);
        if ((sortOrder == Qt::AscendingOrder) ^ lessThan(needNode, node, sort)) {
            begin = row;
            row = (end + begin + 1) / 2;
            if (row >= end)
                break;
        } else {
            end = row;
            row = (end + begin) / 2;
        }
    }

    return row;
}

// 左边比右边小返回true，
bool FileSortWorker::lessThan(const QUrl &left, const QUrl &right, SortScenarios sort)
{
    if (isCanceled)
        return false;

    const auto &leftItem = childrenDataMap.value(left);
    const auto &rightItem = childrenDataMap.value(right);

    const SortInfoPointer leftSortInfo = (leftItem && leftItem->fileSortInfo()) ? leftItem->fileSortInfo() : nullptr;
    const SortInfoPointer rightSortInfo = (rightItem && rightItem->fileSortInfo()) ? rightItem->fileSortInfo() : nullptr;

    if (!leftSortInfo)
        return false;
    if (!rightSortInfo)
        return false;

    bool isDirLeft = leftSortInfo->isDir();
    bool isDirRight = rightSortInfo->isDir();

    // The folder is fixed in the front position
    if (!isMixDirAndFile)
        if (isDirLeft ^ isDirRight)
            return (sortOrder == Qt::DescendingOrder) ^ isDirLeft;

    if (isCanceled)
        return false;

    QVariant leftData = data(leftSortInfo, orgSortRole);
    QVariant rightData = data(rightSortInfo, orgSortRole);

    // 1. 符号链接的大小需要直接获取指向的文件的信息排序
    // 2. 类型排序必须使用 fastMimeType 保证一致性
    bool useFileInfo = false;
    if (!leftData.isValid() || (leftSortInfo->isSymLink())) {
        const FileInfoPointer leftInfo = leftItem && leftItem->fileInfo()
                ? leftItem->fileInfo()
                : InfoFactory::create<FileInfo>(left);
        leftData = data(leftInfo, orgSortRole);
        useFileInfo = true;
    }

    if (!rightData.isValid() || (rightSortInfo->isSymLink())) {
        const FileInfoPointer rightInfo = rightItem && rightItem->fileInfo()
                ? rightItem->fileInfo()
                : InfoFactory::create<FileInfo>(right);
        rightData = data(rightInfo, orgSortRole);
        useFileInfo = true;
    }

    // When the selected sort attribute value is the same, sort by file name
    if (leftData == rightData) {
        QString leftName = leftSortInfo->fileUrl().fileName();
        QString rightName = rightSortInfo->fileUrl().fileName();
        return SortUtils::compareStringForFileName(leftName, rightName);
    }

    switch (orgSortRole) {
    case kItemFileDisplayNameRole:
        return SortUtils::compareStringForFileName(leftData.toString(), rightData.toString());
    case kItemFileLastModifiedRole:
        [[fallthrough]];
    case kItemFileCreatedRole:
        [[fallthrough]];
    case kItemFileDeletionDate:
        [[fallthrough]];
    case kItemFileLastReadRole:
        return SortUtils::compareStringForTime(leftData.toString(), rightData.toString());
    case kItemFileMimeTypeRole:
        return SortUtils::compareStringForMimeType(leftData.toString(), rightData.toString());
    case kItemFileSizeRole:
        // 这里的 useFileInfo 指的是使用 FileInfo 得到的 size 的数据，而非使用 sortFileInfo
        return useFileInfo ? SortUtils::compareForSize(leftData.toLongLong(), rightData.toLongLong())
                           : SortUtils::compareForSize(leftSortInfo, rightSortInfo);
    default:
        return SortUtils::compareStringForFileName(leftData.toString(), rightData.toString());
    }
}

QVariant FileSortWorker::data(const FileInfoPointer &info, ItemRoles role)
{
    if (info.isNull())
        return QVariant();

    auto val = info->customData(role);
    if (val.isValid())
        return val;

    switch (role) {
    case kItemFilePathRole:
        return info->displayOf(DisPlayInfoType::kFileDisplayPath);
    case kItemFileLastModifiedRole: {
        auto lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        return lastModified.isValid() ? lastModified.toString(FileUtils::dateTimeFormat()) : "-";
    }
    case kItemFileCreatedRole: {
        auto created = info->timeOf(TimeInfoType::kCreateTime).value<QDateTime>();
        return created.isValid() ? created.toString(FileUtils::dateTimeFormat()) : "-";
    }
    case kItemIconRole:
        return info->fileIcon();
    case kItemFileSizeRole:
        return info->size();
    case kItemFileMimeTypeRole:
        return info->displayOf(DisPlayInfoType::kFileTypeDisplayName);
    case kItemNameRole:
        return info->nameOf(NameInfoType::kFileName);
    case kItemDisplayRole:
    case kItemEditRole:
    case kItemFileDisplayNameRole:
        return info->displayOf(DisPlayInfoType::kFileDisplayName);
    case kItemFilePinyinNameRole:
        return info->displayOf(DisPlayInfoType::kFileDisplayPinyinName);
    case kItemFileBaseNameRole:
        return info->nameOf(NameInfoType::kCompleteBaseName);
    case kItemFileSuffixRole:
        return info->nameOf(NameInfoType::kSuffix);
    case kItemFileNameOfRenameRole:
        return info->nameOf(NameInfoType::kFileNameOfRename);
    case kItemFileBaseNameOfRenameRole:
        return info->nameOf(NameInfoType::kBaseNameOfRename);
    case kItemFileSuffixOfRenameRole:
        return info->nameOf(NameInfoType::kSuffixOfRename);
    case kItemUrlRole:
        return info->urlOf(UrlInfoType::kUrl);
    default:
        return QVariant();
    }
}

QVariant FileSortWorker::data(const SortInfoPointer &info, Global::ItemRoles role)
{
    auto isContainInHomeDir = [info, role]() {
        if (!info->isDir() || role != kItemFileDisplayNameRole)
            return false;
        const QUrl &url = info->fileUrl();
        static const QString kHomePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        const auto &path = QDir::cleanPath(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile());
        return FileUtils::bindPathTransform(path, false) == kHomePath;
    };

    // 1. 非本地文件的搜索结果不会进行sortinfo的填充，因此直接返回
    // 2. Home 目录下由于 XDG 目录进行了转译，使用 fileinfo 的 displayname 更
    if (info.isNull() || !info->fileUrl().isLocalFile() || isContainInHomeDir())
        return QVariant();

    switch (role) {
    case kItemFileLastReadRole: {
        auto lastRead = QDateTime::fromSecsSinceEpoch(info->lastReadTime());
        return lastRead.isValid() ? lastRead.toString(FileUtils::dateTimeFormat()) : "-";
    }
    case kItemFileLastModifiedRole: {
        auto lastModified = QDateTime::fromSecsSinceEpoch(info->lastModifiedTime());
        return lastModified.isValid() ? lastModified.toString(FileUtils::dateTimeFormat()) : "-";
    }
    case kItemFileDisplayNameRole:
        return info->fileUrl().fileName();
    case kItemFileMimeTypeRole: {
        // perf: MIME type detection cost is high, use caching to reduce calls
        // Avoid large-scale IO operations in CPU-intensive tasks affecting performance
        if (info->customData("fast_mime_type").isValid()) {
            return info->customData("fast_mime_type");
        }
        QString type;
        // For local files, use accurate content-based detection for proper sorting
        type = SortUtils::accurateLocalMimeType(info->fileUrl());
        const_cast<SortInfoPointer &>(info)->setCustomData("fast_mime_type", type);
        return type;
    }
    case kItemFileSizeRole:
        return info->fileSize();
    default:
        return QVariant();
    }
}

bool FileSortWorker::checkFilters(const SortInfoPointer &sortInfo, const bool byInfo)
{
    if (sortInfo.isNull())
        return true;

    auto item = childData(sortInfo->fileUrl());
    if (item && !nameFilters.isEmpty() && !item->data(Global::ItemRoles::kItemFileIsDirRole).toBool()) {
        QRegularExpression re("", QRegularExpression::CaseInsensitiveOption);
        bool hasMatched { false };
        for (int i = 0; i < nameFilters.size(); ++i) {
            QString pattern = QRegularExpression::wildcardToRegularExpression(nameFilters.at(i));
            re.setPattern(pattern);
            if (re.match(item->data(kItemNameRole).toString()).hasMatch()) {
                item->setAvailableState(true);
                hasMatched = true;
                break;
            }
        }
        if (!hasMatched)
            item->setAvailableState(false);
    }

    if (filters == QDir::NoFilter)
        return true;

    const bool isDir = sortInfo->isDir();

    // dir filter
    const bool readable = sortInfo->isReadable();
    const bool writable = sortInfo->isWriteable();
    const bool executable = sortInfo->isExecutable();

    auto checkRWE = [&]() -> bool {
        if ((filters & QDir::Readable) == QDir::Readable) {
            if (!readable)
                return false;
        }
        if ((filters & QDir::Writable) == QDir::Writable) {
            if (!writable)
                return false;
        }
        if ((filters & QDir::Executable) == QDir::Executable) {
            if (!executable)
                return false;
        }
        return true;
    };

    if ((filters & QDir::AllEntries) == QDir::AllEntries
        || ((filters & QDir::Dirs) && (filters & QDir::Files))) {
        // 判断读写执行
        if (!checkRWE())
            return false;
    } else if ((filters & QDir::Dirs) == QDir::Dirs) {
        if (!isDir) {
            return false;
        } else {
            // 判断读写执行
            if (!checkRWE())
                return false;
        }
    } else if ((filters & QDir::Files) == QDir::Files) {
        const bool isFile = sortInfo->isFile();
        if (!isFile) {
            return false;
        } else {
            // 判断读写执行
            if (!checkRWE())
                return false;
        }
    }

    if ((filters & QDir::NoSymLinks) == QDir::NoSymLinks) {
        const bool isSymlinks = sortInfo->isSymLink();
        if (isSymlinks)
            return false;
    }

    const bool showHidden = (filters & QDir::Hidden) == QDir::Hidden;
    if (!showHidden) {   // hide files
        bool isHidden = sortInfo->isHide();
        // /mount-point/root, /mount-point/lost+found of LOCAL disk should be treat as hidden file.
        if (isHidden || isDefaultHiddenFile(sortInfo->fileUrl()))
            return false;
    }

    if (filterCallback)
        return filterCallback(sortInfo.data(), filterData);

    return true;
}

bool FileSortWorker::isDefaultHiddenFile(const QUrl &fileUrl)
{
    static DThreadList<QUrl> defaultHiddenUrls;
    static std::once_flag flg;
    std::call_once(flg, [&] {
        using namespace GlobalServerDefines;
        const auto &systemBlks = DevProxyMng->getAllBlockIds(DeviceQueryOption::kSystem | DeviceQueryOption::kMounted);
        for (const auto &blk : systemBlks) {
            auto blkInfo = DevProxyMng->queryBlockInfo(blk);
            const QStringList &mountPoints = blkInfo.value(DeviceProperty::kMountPoints).toStringList();
            for (const auto &mpt : mountPoints) {
                defaultHiddenUrls.push_backByLock(QUrl::fromLocalFile(mpt + (mpt == "/" ? "root" : "/root")));
                defaultHiddenUrls.push_backByLock(QUrl::fromLocalFile(mpt + (mpt == "/" ? "lost+found" : "/lost+found")));
            }
        }
    });
    return defaultHiddenUrls.containsByLock(fileUrl);
}

QUrl FileSortWorker::makeParentUrl(const QUrl &url)
{
    if (!currentSupportTreeView || !istree)
        return current;

    auto parent = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
    if (UniversalUtils::urlEquals(current, parent) || UniversalUtils::isParentUrl(parent, current)
        || !childData(parent).isNull())
        return parent;

    return current;
}

int8_t FileSortWorker::getDepth(const QUrl &url)
{
    for (const auto &key : depthMap.keys()) {
        for (const auto &value : depthMap.values(key)) {
            if (UniversalUtils::urlEquals(url, value))
                return key;
        }
    }
    return -2;
}

int FileSortWorker::findRealShowIndex(const QUrl &preItemUrl)
{
    const FileItemDataPointer &preItemPtr = childrenDataMap.value(preItemUrl, nullptr);
    if (!preItemPtr || !preItemPtr->data(Global::ItemRoles::kItemTreeViewExpandedRole).toBool())
        return indexOfVisibleChild(preItemUrl) + 1;

    QList<QUrl> preSubItemList = visibleTreeChildren.value(preItemUrl, {});
    if (preSubItemList.isEmpty())
        return indexOfVisibleChild(preItemUrl) + 1;

    const QUrl &subPreItemUrl = preSubItemList.last();
    return findRealShowIndex(subPreItemUrl);
}

int FileSortWorker::indexOfVisibleChild(const QUrl &itemUrl)
{
    QReadLocker lk(&locker);
    return visibleChildren.indexOf(itemUrl);
}

int FileSortWorker::setVisibleChildren(const int startPos, const QList<QUrl> &filterUrls, const FileSortWorker::InsertOpt opt, const int endPos)
{
    QList<QUrl> visibleList;
    if (opt == InsertOpt::kInsertOptForce) {
        visibleList = filterUrls;
    } else {
        auto tmp = getChildrenUrls();
        visibleList.append(tmp.mid(0, startPos));
        visibleList.append(filterUrls);
        if (opt == InsertOpt::kInsertOptReplace) {
            visibleList.append(tmp.mid(endPos != -1 ? endPos : startPos + filterUrls.length()));
        } else if (opt == InsertOpt::kInsertOptAppend) {
            visibleList.append(tmp.mid(startPos));
        }
    }

    if (isCanceled)
        return -1;

    QWriteLocker lk(&locker);
    visibleChildren = visibleList;

    return visibleList.length();
}

bool FileSortWorker::checkAndUpdateFileInfoUpdate()
{
    if (sortRole != DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault || !mimeSorting)
        return true;

    QList<FileItemDataPointer> items;
    {
        QReadLocker lk(&childrenDataLocker);
        items = childrenDataMap.values();
    }

    for (auto item : items) {
        if (!mimeSorting) {
            waitUpdatedFiles.clear();
            if (isCanceled)
                emit reqUestCloseCursor();
            return !isCanceled;
        }
        auto info = item->fileInfo();
        if (info.isNull() || !info->extendAttributes(ExtInfoType::kFileNeedTransInfo).toBool())
            continue;
        if (!info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool())
            waitUpdatedFiles.insert(info->fileUrl());
        item->data(Global::ItemRoles::kItemUpdateAndTransFileInfoRole);
    }

    if (!waitUpdatedFiles.isEmpty())
        return false;

    return true;
}

void FileSortWorker::checkAndSortBytMimeType(const QUrl &url)
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    if (!mimeSorting || isCanceled)
        return;
    if (waitUpdatedFiles.contains(url))
        waitUpdatedFiles.remove(url);

    if (waitUpdatedFiles.count() <= 0) {
        mimeSorting = false;
        emit requestSortByMimeType();
    }
}

void FileSortWorker::doCompleteFileInfo(SortInfoPointer sortInfo)
{
    if (!sortInfo || sortInfo->isInfoCompleted())
        return;

    QUrl url = sortInfo->fileUrl();

    if (!url.isLocalFile())
        return;

    struct stat64 statBuffer;
    const QString filePath = url.path();

    if (::stat64(filePath.toUtf8().constData(), &statBuffer) != 0)
        return;

    // 一次性设置所有从 stat64 获取的信息

    // 基础信息
    sortInfo->setSize(statBuffer.st_size);
    sortInfo->setFile(S_ISREG(statBuffer.st_mode));
    sortInfo->setDir(S_ISDIR(statBuffer.st_mode));
    sortInfo->setSymlink(S_ISLNK(statBuffer.st_mode));

    // 隐藏文件检查
    QString fileName = url.fileName();
    sortInfo->setHide(fileName.startsWith('.'));

    // 权限信息
    sortInfo->setReadable(statBuffer.st_mode & S_IRUSR);
    sortInfo->setWriteable(statBuffer.st_mode & S_IWUSR);
    sortInfo->setExecutable(statBuffer.st_mode & S_IXUSR);

    // 时间信息
    sortInfo->setLastReadTime(statBuffer.st_atime);
    sortInfo->setLastModifiedTime(statBuffer.st_mtime);
    sortInfo->setCreateTime(statBuffer.st_ctime);

    // 标记所有信息已完成
    sortInfo->setInfoCompleted(true);
}

QList<FileItemDataPointer> FileSortWorker::getAllFiles() const
{
    QList<FileItemDataPointer> allFiles;

    QReadLocker lk(&childrenDataLocker);
    QReadLocker vlk(&locker);

    // 在 TeeView 下使用 visibleChildren 将导致子目录的文件也被分组
    const QList<QUrl> &children =
            visibleTreeChildren.contains(current)
            ? visibleTreeChildren[current]
            : visibleChildren;

    for (const QUrl &url : children) {
        if (childrenDataMap.contains(url)) {
            const FileItemDataPointer &fileData = childrenDataMap.value(url);
            if (fileData) {
                allFiles.append(fileData);
            }
        }
    }

    fmDebug() << "FileSortWorker: Retrieved" << allFiles.size() << "files for grouping";
    return allFiles;
}

void FileSortWorker::applyGrouping(const QList<FileItemDataPointer> &files)
{
    Q_ASSERT(currentStrategy);
    Q_ASSERT(groupingEngine);

    if (files.isEmpty() || isCanceled)
        return;

    emit requestCursorWait();
    FinallyUtil release([this] {
        emit reqUestCloseCursor();
    });

    // Perform grouping using GroupingEngine
    groupingEngine->setGroupOrder(groupOrder);
    groupingEngine->setVisibleTreeChildren(&visibleTreeChildren);
    groupingEngine->setChildrenDataMap(&childrenDataMap);
    groupingEngine->setVisibleChildren(&visibleChildren);

    const auto &result = groupingEngine->groupFiles(files, currentStrategy);
    if (!result.success) {
        fmCritical() << "FileSortWorker: Grouping failed:" << result.errorMessage;
        return;
    }

    // Generate model data with current expansion states
    const auto &data = groupingEngine->generateModelData(result, groupExpansionStates);
    doModelChanged(ModelChangeType::kInsertGroupRows, 0, data.getItemCount());
    groupedModelData = data;
    doModelChanged(ModelChangeType::kInsertGroupFinished);
}

void FileSortWorker::clearGroupedData()
{
    doModelChanged(ModelChangeType::kRemoveGroupRows, 0, groupedModelData.getItemCount());
    groupedModelData.clear();
    doModelChanged(ModelChangeType::kRemoveGroupFinished);
}

int FileSortWorker::childrenCountInternal()
{
    QReadLocker lk(&locker);
    return visibleChildren.count();
}

int FileSortWorker::getChildShowIndexInternal(const QUrl &url)
{
    QReadLocker lk(&locker);
    return visibleChildren.indexOf(url);
}

void FileSortWorker::doModelChanged(const ModelChangeType type, int index, int count)
{
    if (currentIsGroupingMode()) {
        switch (type) {
        // 收到普通插入/删除事件，但需要按分组逻辑处理
        case ModelChangeType::kInsertRows:
            handleAboutToInsertFilesToGroup(index, count);
            break;
        case ModelChangeType::kRemoveRows:
            handleAboutToRemoveFilesFromGroup(index, count);
            break;
        // 插入/删除完成事件，在分组模式下触发通用的变更处理
        case ModelChangeType::kInsertFinished:
        case ModelChangeType::kRemoveFinished:
            handleGroupingChanged();
            break;
        // 直接转发专门用于分组视图的事件
        case ModelChangeType::kInsertGroupRows:
            Q_EMIT insertGroupRows(index, count);
            break;
        case ModelChangeType::kInsertGroupFinished:
            Q_EMIT insertGroupFinish();
            break;
        case ModelChangeType::kRemoveGroupRows:
            Q_EMIT removeGroupRows(index, count);
            break;
        case ModelChangeType::kRemoveGroupFinished:
            Q_EMIT removeGroupFinish();
            break;

        default:
            break;
        }
    } else {
        // 在非分组模式下，我们只关心“普通”事件，忽略所有“Group”事件
        switch (type) {
        case ModelChangeType::kInsertRows:
            Q_EMIT insertRows(index, count);
            break;
        case ModelChangeType::kInsertFinished:
            Q_EMIT insertFinish();
            break;
        case ModelChangeType::kRemoveRows:
            Q_EMIT removeRows(index, count);
            break;
        case ModelChangeType::kRemoveFinished:
            Q_EMIT removeFinish();
            break;
        default:
            break;
        }
    }
}
