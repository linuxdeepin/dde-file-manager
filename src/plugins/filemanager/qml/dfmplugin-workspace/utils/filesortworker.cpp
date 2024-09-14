// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesortworker.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include "workspacehelper.h"

#include <dfm-io/dfmio_utils.h>

#include <QStandardPaths>
#include <QRegularExpression>

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;
using namespace dfmio;

FileSortWorker::FileSortWorker(const QUrl &url, const QString &key, FileViewFilterCallback callfun, const QStringList &nameFilters, const QDir::Filters filters, const QDirIterator::IteratorFlags flags, QObject *parent)
    : QObject(parent), current(url), nameFilters(nameFilters), filters(filters), flags(flags), filterCallback(callfun), currentKey(key)
{
    auto dirPath = url.path();
    if (!dirPath.isEmpty() && dirPath != QDir::separator() && url.path().endsWith(QDir::separator()))
        dirPath.chop(1);
    current.setPath(dirPath);
    sortAndFilter = SortFilterFactory::create<AbstractSortFilter>(current);
    isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FileSortWorker::handleFileInfoUpdated, Qt::QueuedConnection);
    currentSupportTreeView = WorkspaceHelper::instance()->supportTreeView(current.scheme());
}

FileSortWorker::~FileSortWorker()
{
    isCanceled = true;
    childrenDataMap.clear();
    visibleChildren.clear();
    children.clear();
    if (updateRefresh) {
        updateRefresh->stop();
        updateRefresh->deleteLater();
        updateRefresh = nullptr;
    }
}

FileSortWorker::SortOpt FileSortWorker::setSortAgruments(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile)
{
    FileSortWorker::SortOpt opt { FileSortWorker::SortOpt::kSortOptNone };
    if (sortOrder == order && orgSortRole == sortRole && this->isMixDirAndFile == isMixDirAndFile)
        return opt;
    if (orgSortRole != sortRole || this->isMixDirAndFile != isMixDirAndFile) {
        opt = FileSortWorker::SortOpt::kSortOptOtherChanged;
    } else {
        opt = FileSortWorker::SortOpt::kSortOptOnlyOrderChanged;
    }

    sortOrder = order;
    orgSortRole = sortRole;
    this->isMixDirAndFile = isMixDirAndFile;
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

QUrl FileSortWorker::mapToIndex(int index)
{
    QReadLocker lk(&locker);

    if (index < 0 || index >= visibleChildren.count())
        return QUrl();
    return visibleChildren.at(index);
}

int FileSortWorker::childrenCount()
{
    QReadLocker lk(&locker);
    return visibleChildren.count();
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
    QUrl url;
    {
        QReadLocker lk(&locker);
        if (index < 0 || index >= visibleChildren.count())
            return nullptr;
        url = visibleChildren.at(index);
    }

    QReadLocker lk(&childrenDataLocker);
    return childrenDataMap.value(url);
}

void FileSortWorker::cancel()
{
    isCanceled = true;
}

int FileSortWorker::getChildShowIndex(const QUrl &url)
{
    QReadLocker lk(&locker);
    return visibleChildren.indexOf(url);
}

QList<QUrl> FileSortWorker::getChildrenUrls()
{
    QReadLocker lk(&locker);
    return visibleChildren;
}

QDir::Filters FileSortWorker::getFilters() const
{
    return filters;
}

ItemRoles FileSortWorker::getSortRole() const
{
    return orgSortRole;
}

Qt::SortOrder FileSortWorker::getSortOrder() const
{
    return sortOrder;
}

void FileSortWorker::setTreeView(const bool isTree)
{
    istree = isTree;
    isMixDirAndFile = istree ? false : isMixDirAndFile;
}

void FileSortWorker::handleIteratorLocalChildren(const QString &key,
                                                 const QList<SortInfoPointer> children,
                                                 const DEnumerator::SortRoleCompareFlag sortRole,
                                                 const Qt::SortOrder sortOrder,
                                                 const bool isMixDirAndFile)
{
    handleAddChildren(key, children, {}, sortRole, sortOrder, isMixDirAndFile, false, false);
}

void FileSortWorker::handleSourceChildren(const QString &key,
                                          const QList<SortInfoPointer> children,
                                          const DEnumerator::SortRoleCompareFlag sortRole,
                                          const Qt::SortOrder sortOrder, const bool isMixDirAndFile,
                                          const bool isFinished)
{
    handleAddChildren(key, children, {}, sortRole, sortOrder, isMixDirAndFile, true, isFinished);
}

void FileSortWorker::handleIteratorChildren(const QString &key, const QList<SortInfoPointer> children, const QList<FileInfoPointer> infos)
{
    handleAddChildren(key, children, infos, sortRole, sortOrder, isMixDirAndFile, false, false, false);
}

void FileSortWorker::handleTraversalFinish(const QString &key)
{
    if (currentKey != key)
        return;

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

void FileSortWorker::handleModelGetSourceData()
{
    if (isCanceled)
        return;
    emit getSourceData(currentKey);
}

void FileSortWorker::handleFilters(QDir::Filters filters)
{
    resetFilters(filters);
}

void FileSortWorker::HandleNameFilters(const QStringList &filters)
{
    nameFilters = filters;
    QMap<QUrl, FileItemDataPointer>::iterator itr = childrenDataMap.begin();
    for (; itr != childrenDataMap.end(); ++itr) {
        checkNameFilters(itr.value());
    }
    Q_EMIT requestUpdateView();
}

void FileSortWorker::handleFilterData(const QVariant &data)
{
    if (isCanceled)
        return;

    filterData = data;
    if (!filterCallback || !data.isValid())
        return;

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
    bool added = false;
    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;
        if (this->children.value(parantUrl(sortInfo->fileUrl())).contains(sortInfo->fileUrl())) {
            auto data = childData(sortInfo->fileUrl());
            if (data && data->fileInfo())
                data->fileInfo()->updateAttributes();
            continue;
        }
        auto suc = addChild(sortInfo, AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile);
        if (!added)
            added = suc;
    }

    if (added) {
        qWarning() << "insert 111111111";
        Q_EMIT insertFinish();
    }
}

void FileSortWorker::handleWatcherRemoveChildren(const QList<SortInfoPointer> &children)
{
    if (children.isEmpty())
        return;
    auto parentUrl = parantUrl(children.first()->fileUrl());

    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;

        if (sortInfo.isNull())
            continue;

        if (sortInfo->isDir() && visibleTreeChildren.keys().contains(sortInfo->fileUrl())) {
            removeSubDir(sortInfo->fileUrl());
            continue;
        }
    }

    auto subChildren = this->children.take(parentUrl);
    auto subVisibleList = visibleTreeChildren.take(parentUrl);
    bool removed = false;
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

        Q_EMIT removeRows(showIndex, 1);
        removed = true;
        {
            QWriteLocker lk(&locker);
            visibleChildren.removeAt(showIndex);
        }
    }
    if (removed)
        Q_EMIT removeFinish();
    this->children.insert(parentUrl, subChildren);
    visibleTreeChildren.insert(parentUrl, subVisibleList);
}

bool FileSortWorker::handleWatcherUpdateFile(const SortInfoPointer child)
{
    if (isCanceled)
        return false;

    if (!child)
        return false;

    if (!child->fileUrl().isValid() || !this->children.value(parantUrl(child->fileUrl())).contains(child->fileUrl()))
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
    bool added = false;
    for (auto sort : children) {
        if (isCanceled)
            return;
        auto suc = handleWatcherUpdateFile(sort);
        if (!added)
            added = suc;
    }

    if (added) {
        qWarning() << "insert 2222222222";
        Q_EMIT insertFinish();
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
    auto parentUrl = parantUrl(hidUrl);
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
    if (isCanceled)
        return;

    auto opt = setSortAgruments(order, sortRole, /*istree ? false :*/ isMixDirAndFile);
    switch (opt) {
    case FileSortWorker::SortOpt::kSortOptOtherChanged:
        return resortCurrent(false);
    case FileSortWorker::SortOpt::kSortOptOnlyOrderChanged:
        return resortCurrent(true);
    default:
        return;
    }
}

void FileSortWorker::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (isCanceled || istree)
        return;

    if (aa == Application::kFileAndDirMixedSort)
        handleResort(sortOrder, orgSortRole, value.toBool());
}

bool FileSortWorker::handleUpdateFile(const QUrl &url)
{
    if (isCanceled)
        return false;

    if (!url.isValid())
        return false;

    SortInfoPointer sortInfo = children.value(parantUrl(url)).value(url);
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
            Q_EMIT removeRows(childIndex, 1);
            {
                QWriteLocker lk(&locker);
                visibleChildren.removeAt(childIndex);
            }
            Q_EMIT removeFinish();
            return false;
        }
        Q_EMIT updateRow(childIndex);
        return false;
    }

    bool added = false;
    if (checkFilters(sortInfo, true)) {
        auto parentUrl = parantUrl(sortInfo->fileUrl());
        int showIndex = findStartPos(parentUrl);

        // 插入到每个目录下的显示目录
        auto subVisibleList = visibleTreeChildren.take(parentUrl);
        auto offset = subVisibleList.length();
        if (orgSortRole != Global::ItemRoles::kItemDisplayRole)
            offset = insertSortList(sortInfo->fileUrl(), subVisibleList, AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile);
        auto subIndex = offset;
        // 根目录下的offset计算不一样
        if (UniversalUtils::urlEquals(parentUrl, current)) {
            if (offset >= subVisibleList.length() || offset == 0) {
                offset = offset >= subVisibleList.length() ? childrenCount() : 0;
            } else {
                auto tmpUrl = offset >= subVisibleList.length() ? QUrl() : subVisibleList.at(offset);
                offset = getChildShowIndex(tmpUrl);
                if (offset < 0)
                    offset = childrenCount();
            }
        }
        subVisibleList.insert(subIndex, sortInfo->fileUrl());
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

        Q_EMIT insertRows(showIndex, 1);
        {
            QWriteLocker lk(&locker);
            visibleChildren.insert(showIndex, sortInfo->fileUrl());
        }
        added = true;

        // async create file will add to view while file info updated.
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());
    }

    return added;
}

void FileSortWorker::handleUpdateFiles(const QList<QUrl> &urls)
{
    bool added = false;
    for (auto const &url : urls) {
        if (isCanceled)
            return;
        auto suc = handleUpdateFile(url);
        if (!added)
            added = suc;
    }
    if (added) {
        qWarning() << "insert 33333333333";
        emit insertFinish();
    }
}

void FileSortWorker::handleRefresh()
{
    int childrenCount = this->childrenCount();
    if (childrenCount > 0)
        Q_EMIT removeRows(0, childrenCount);

    {
        QWriteLocker lk(&locker);
        visibleChildren.clear();
    }
    children.clear();
    visibleTreeChildren.clear();
    depthMap.clear();

    {
        QWriteLocker lk(&childrenDataLocker);
        childrenDataLastMap = childrenDataMap;
        childrenDataMap.clear();
    }

    if (childrenCount > 0)
        Q_EMIT removeFinish();

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
    if (!children.value(parantUrl(url)).contains(url))
        return;

    auto itemdata = childData(url);
    if (!itemdata)
        return;

    auto fileInfo = itemdata->fileInfo();
    if (!fileInfo || QString::number(quintptr(fileInfo.data()), 16) != infoPtr)
        return;

    fileInfo->customData(Global::ItemRoles::kItemFileRefreshIcon);

    sortInfoUpdateByFileInfo(fileInfo);

    if (fileInfoRefresh.contains(url))
        return;

    fileInfoRefresh.append(url);

    if (updateRefresh && updateRefresh->isActive())
        return;

    if (!updateRefresh) {
        updateRefresh = new QTimer;
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
    if (isTree == istree)
        return;
    istree = isTree;
    if (istree) {
        switchTreeView();
    } else {
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
                                       const bool isSort)
{
    if (!handleAddChildren(key, children, childInfos))
        return;

    if (children.isEmpty()) {
        if (handleSource)
            setSourceHandleState(isFinished);

        return;
    }

    // In the home, it is necessary to sort by display name.
    // So, using `sortAllFiles` to reorder
    auto parentUrl = parantUrl(children.first()->fileUrl());
    bool isHome = parentUrl.path() == StandardPaths::location(StandardPaths::kHomePath);
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

// 这个函数处理的是当前新增加的
bool FileSortWorker::handleAddChildren(const QString &key,
                                       const QList<SortInfoPointer> &children,
                                       const QList<FileInfoPointer> &childInfos)
{
    if (currentKey != key || isCanceled)
        return false;
    if (children.isEmpty())
        return true;

    // 获取相对于已有的新增加的文件
    QList<QUrl> newChildren;
    childrenDataLastMap.clear();

    auto parentUrl = parantUrl(children.first()->fileUrl());
    // 获取当前的插入的位置
    auto childUrls = visibleTreeChildren.take(parentUrl);
    auto startPos = findStartPos(parentUrl);
    auto posOffset = childUrls.length();
    QMap<QUrl, SortInfoPointer> tmpChildren = this->children.take(parentUrl);
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

    return false;
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

    for (int i = 0; i < nameFilters.size(); ++i) {
        QRegularExpression re(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(nameFilters.at(i))));
        QRegularExpressionMatchIterator it = re.globalMatch(itemData->data(kItemNameRole).toString());
        if (it.hasNext()) {
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
            auto sortInfo = children.value(parantUrl(parent)).value(parent);
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

        if (checkFilters(sortInfo, byInfo))
            filterUrls.append(sortInfo->fileUrl());
    }

    visibleTreeChildren.remove(parent);
    if (filterUrls.isEmpty()) {
        if (UniversalUtils::urlEquals(parent, current)) {
            Q_EMIT removeRows(0, visibleChildren.count());
            QWriteLocker lk(&locker);
            visibleChildren.clear();
            Q_EMIT removeFinish();
        }
        return;
    }

    visibleTreeChildren.insert(parent, filterUrls);
}

bool FileSortWorker::addChild(const SortInfoPointer &sortInfo,
                              const AbstractSortFilter::SortScenarios sort)
{
    if (isCanceled || sortInfo.isNull())
        return false;

    auto parentUrl = parantUrl(sortInfo->fileUrl());
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
            offset = offset >= subVisibleList.length() ? childrenCount() : 0;
        } else {
            auto tmpUrl = offset >= subVisibleList.length() ? QUrl() : subVisibleList.at(offset);
            offset = getChildShowIndex(tmpUrl);
            if (offset < 0)
                offset = childrenCount();
        }
    }
    subVisibleList.insert(subIndex, sortInfo->fileUrl());
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

    Q_EMIT insertRows(showIndex, 1);
    {
        QWriteLocker lk(&locker);
        visibleChildren.insert(showIndex, sortInfo->fileUrl());
    }

    if (sort == AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile)
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());

    return true;
}

bool FileSortWorker::sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo)
{
    if (!fileInfo)
        return false;

    auto url = fileInfo->fileUrl();
    auto parentUrl = parantUrl(url);
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
    fileInfo->fileMimeType();

    return true;
}

void FileSortWorker::switchTreeView()
{
    // 当前只有一层，只需要展开获取每个目录的展开属性,只有父母这一层
    if (isMixDirAndFile)
        handleResort(sortOrder, orgSortRole, false);
    emit requestUpdateView();
}

void FileSortWorker::switchListView()
{
    // 移除depthMap和visibleTreeChildren
    auto allShowList = visibleTreeChildren.value(current);
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

    auto parentUrl = parantUrl(children.first());
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
    QMap<QUrl, SortInfoPointer> sortInfos = reverse && !isMixDirAndFile ? this->children.value(parentUrl)
                                                                        : QMap<QUrl, SortInfoPointer>();
    bool firstFile = false;
    for (const auto &url : children) {
        if (isCanceled)
            return {};
        if (!reverse) {
            sortIndex = insertSortList(url, sortList, AbstractSortFilter::SortScenarios::kSortScenariosNormal);
        } else if (!firstFile && !isMixDirAndFile) {
            auto sortInfo = sortInfos.value(url);
            if (sortInfo && sortInfo->isFile()) {
                firstFile = true;
                sortIndex = sortList.count();
            }
        }
        sortList.insert(sortIndex, url);
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
    removeVisibleChildren(startPos, endPos == -1 ? childrenCount() - startPos : endPos - startPos);
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
        return childrenCount();

    const auto &parentUrl = parantUrl(dir);
    auto index = visibleTreeChildren.value(parentUrl).indexOf(dir);
    if (index < 0)
        return -1;

    if (index == visibleTreeChildren.value(parentUrl).length() - 1)
        return findEndPos(parantUrl(dir));

    return getChildShowIndex(visibleTreeChildren.value(parentUrl).at(index + 1));
}

int FileSortWorker::findStartPos(const QUrl &parent)
{
    if (UniversalUtils::urlEquals(parent, current))
        return 0;
    auto pos = getChildShowIndex(parent);
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

    Q_EMIT insertRows(startPos, filterUrls.length());
    setVisibleChildren(startPos, filterUrls, opt, endPos);
    Q_EMIT insertFinish();
}

void FileSortWorker::removeVisibleChildren(const int startPos, const int size)
{
    if (isCanceled || size <= 0)
        return;
    Q_EMIT removeRows(startPos, size);
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

    Q_EMIT removeFinish();
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

    if (!istree || !child->isDir()) {
        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.insert(child->fileUrl(), item);
        return;
    }

    QWriteLocker lk(&childrenDataLocker);
    childrenDataMap.insert(child->fileUrl(), item);
}

int FileSortWorker::insertSortList(const QUrl &needNode, const QList<QUrl> &list,
                                   AbstractSortFilter::SortScenarios sort)
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
bool FileSortWorker::lessThan(const QUrl &left, const QUrl &right, AbstractSortFilter::SortScenarios sort)
{
    if (isCanceled)
        return false;

    const auto &leftItem = childrenDataMap.value(left);
    const auto &rightItem = childrenDataMap.value(right);

    const FileInfoPointer leftInfo = leftItem && leftItem->fileInfo()
            ? leftItem->fileInfo()
            : InfoFactory::create<FileInfo>(left);
    const FileInfoPointer rightInfo = rightItem && rightItem->fileInfo()
            ? rightItem->fileInfo()
            : InfoFactory::create<FileInfo>(right);

    if (!leftInfo)
        return false;
    if (!rightInfo)
        return false;

    if (sortAndFilter) {
        auto result = sortAndFilter->lessThan(leftInfo, rightInfo, isMixDirAndFile,
                                              orgSortRole, sort);
        if (result > 0)
            return result;
    }

    bool isDirLeft = leftInfo->isAttributes(OptInfoType::kIsDir);
    bool isDirRight = rightInfo->isAttributes(OptInfoType::kIsDir);

    // The folder is fixed in the front position
    if (!isMixDirAndFile)
        if (isDirLeft ^ isDirRight)
            return (sortOrder == Qt::DescendingOrder) ^ isDirLeft;

    if (isCanceled)
        return false;

    QVariant leftData = data(leftInfo, orgSortRole);
    QVariant rightData = data(rightInfo, orgSortRole);

    // When the selected sort attribute value is the same, sort by file name
    if (leftData == rightData) {
        QString leftName = leftInfo->displayOf(DisPlayInfoType::kFileDisplayName);
        QString rightName = rightInfo->displayOf(DisPlayInfoType::kFileDisplayName);
        return FileUtils::compareByStringEx(leftName, rightName);
    }

    switch (orgSortRole) {
    case kItemFileDisplayNameRole:
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
        return FileUtils::compareByStringEx(leftData.toString(), rightData.toString());
    case kItemFileSizeRole: {
        qint64 sizel = leftInfo->size();
        qint64 sizer = rightInfo->size();
        return sizel < sizer;
    }
    default:
        return FileUtils::compareByStringEx(leftData.toString(), rightData.toString());
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
    case kItemIconRole:
        return info->fileIcon();
    case kItemFileSizeRole:
        return info->displayOf(DisPlayInfoType::kSizeDisplayName);
    case kItemFileMimeTypeRole:
        return info->displayOf(DisPlayInfoType::kMimeTypeDisplayName);
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

bool FileSortWorker::checkFilters(const SortInfoPointer &sortInfo, const bool byInfo)
{
    auto item = childData(sortInfo->fileUrl());
    if (item && !nameFilters.isEmpty() && !item->data(Global::ItemRoles::kItemFileIsDirRole).toBool()) {
        QRegularExpression re("", QRegularExpression::CaseInsensitiveOption);
        for (int i = 0; i < nameFilters.size(); ++i) {
            re.setPattern(nameFilters.at(i));
            if (re.match(item->data(kItemNameRole).toString()).hasMatch()) {
                item->setAvailableState(true);
            }
        }
    }

    // 处理继承
    if (sortInfo && sortAndFilter) {
        auto result = sortAndFilter->checkFilters(InfoFactory::create<FileInfo>(sortInfo->fileUrl()), filters, filterData);
        if (result >= 0)
            return result;
    }

    if (!sortInfo || filters == QDir::NoFilter)
        return true;

    FileInfoPointer fileInfo { nullptr };

    if (byInfo) {
        auto itemdata = childData(sortInfo->fileUrl());
        fileInfo = itemdata ? itemdata->fileInfo() : InfoFactory::create<FileInfo>(sortInfo->fileUrl());
    }

    if (fileInfo && !fileInfo->exists())
        return false;

    const bool isDir = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsDir) : sortInfo->isDir();

    // dir filter
    const bool readable = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsReadable) : sortInfo->isReadable();
    const bool writable = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsWritable) : sortInfo->isWriteable();
    const bool executable = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsExecutable) : sortInfo->isExecutable();

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
        const bool isSymlinks = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsSymLink) : sortInfo->isSymLink();
        if (isSymlinks)
            return false;
    }

    const bool showHidden = (filters & QDir::Hidden) == QDir::Hidden;
    if (!showHidden) {   // hide files
        bool isHidden = fileInfo ? fileInfo->isAttributes(OptInfoType::kIsHidden) : sortInfo->isHide();
        // /mount-point/root, /mount-point/lost+found of LOCAL disk should be treat as hidden file.
        if (isHidden || isDefaultHiddenFile(fileInfo ? fileInfo->urlOf(UrlInfoType::kUrl) : sortInfo->fileUrl()))
            return false;
    }

    // all dir, don't apply the filters to directory names.
    if ((filters & QDir::AllDirs) == QDir::AllDirs) {
        if (isDir)
            return true;
    }

    if (sortInfo && filterCallback)
        return filterCallback(InfoFactory::create<FileInfo>(sortInfo->fileUrl()).data(), filterData);

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
                defaultHiddenUrls.push_back(QUrl::fromLocalFile(mpt + (mpt == "/" ? "root" : "/root")));
                defaultHiddenUrls.push_back(QUrl::fromLocalFile(mpt + (mpt == "/" ? "lost+found" : "/lost+found")));
            }
        }
    });
    return defaultHiddenUrls.contains(fileUrl);
}

QUrl FileSortWorker::parantUrl(const QUrl &url)
{
    if (!currentSupportTreeView)
        return current;
    auto parent = UrlRoute::urlParent(url);
    if (UniversalUtils::urlEquals(current, parent) || UniversalUtils::isParentUrl(current, parent)
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
