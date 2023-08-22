// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesortworker.h"
#include "models/fileitemdata.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/base/standardpaths.h>

#include <dfm-io/dfmio_utils.h>

#include <QStandardPaths>

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;
using namespace dfmio;

FileSortWorker::FileSortWorker(const QUrl &url, const QString &key, FileViewFilterCallback callfun, const QStringList &nameFilters, const QDir::Filters filters, const QDirIterator::IteratorFlags flags, QObject *parent)
    : QObject(parent), current(url), nameFilters(nameFilters), filters(filters), flags(flags), filterCallback(callfun), currentKey(key)
{
    sortAndFilter = SortFilterFactory::create<AbstractSortFilter>(url);
    isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FileSortWorker::handleFileInfoUpdated, Qt::QueuedConnection);
}

FileSortWorker::~FileSortWorker()
{
    isCanceled = true;
    if (rootdata) {
        rootdata = nullptr;
        delete rootdata;
    }
    qDeleteAll(childrenDataMap.values());
    childrenDataMap.clear();
    childrenUrlList.clear();
    visibleChildren.clear();
    children.clear();
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

FileItemData *FileSortWorker::childData(const QUrl &url)
{
    QReadLocker lk(&childrenDataLocker);
    return childrenDataMap.value(url);
}

void FileSortWorker::setRootData(FileItemData *data)
{
    rootdata = data;
}

FileItemData *FileSortWorker::rootData() const
{
    return rootdata;
}

FileItemData *FileSortWorker::childData(const int index)
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

void FileSortWorker::handleIteratorLocalChildren(const QString &key,
                                                 QList<SortInfoPointer> children,
                                                 const DEnumerator::SortRoleCompareFlag sortRole,
                                                 const Qt::SortOrder sortOrder,
                                                 const bool isMixDirAndFile)
{
    if (currentKey != key)
        return;

    this->children = children;
    for (const auto &child : children) {
        childrenUrlList.append(child->fileUrl());
        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.insert(child->fileUrl(), new FileItemData(child, rootdata));
    }

    if (isCanceled)
        return;

    filterAllFiles();

    // In the home, it is necessary to sort by display name.
    // So, using `sortAllFiles` to reorder
    bool isHome = current.path() == StandardPaths::location(StandardPaths::kHomePath);
    if (!isHome && sortRole != DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault && this->sortRole == sortRole
        && this->sortOrder == sortOrder && this->isMixDirAndFile == isMixDirAndFile)
        return;

    if (isCanceled)
        return;

    sortAllFiles();
}

void FileSortWorker::handleSourceChildren(const QString &key,
                                          QList<SortInfoPointer> children,
                                          const DEnumerator::SortRoleCompareFlag sortRole,
                                          const Qt::SortOrder sortOrder, const bool isMixDirAndFile,
                                          const bool isFinished)
{
    if (currentKey != key)
        return;

    if (this->childrenUrlList.isEmpty()) {
        handleIteratorLocalChildren(key, children, sortRole, sortOrder, isMixDirAndFile);
        if (isFinished) {
            Q_EMIT requestSetIdel();
        } else {
            Q_EMIT getSourceData(currentKey);
        }
        return;
    }

    // 获取相对于已有的新增加的文件
    QList<QUrl> newChildren;
    for (const auto &sortInfo : children) {
        if (this->childrenUrlList.contains(sortInfo->fileUrl()))
            continue;
        this->children.append(sortInfo);
        this->childrenUrlList.append(sortInfo->fileUrl());
        {
            QWriteLocker lk(&childrenDataLocker);
            childrenDataMap.insert(sortInfo->fileUrl(), new FileItemData(sortInfo, rootdata));
        }
        if (checkFilters(sortInfo))
            newChildren.append(sortInfo->fileUrl());
        if (isCanceled)
            return;
    }

    if (sortRole != DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault && this->sortRole == sortRole
        && this->sortOrder == sortOrder && this->isMixDirAndFile == isMixDirAndFile) {
        Q_EMIT insertRows(visibleChildren.length(), newChildren.length());
        {
            QWriteLocker lk(&locker);
            visibleChildren.append(newChildren);
        }
        Q_EMIT insertFinish();

        if (isFinished) {
            Q_EMIT requestSetIdel();
        } else {
            Q_EMIT getSourceData(currentKey);
        }
        return;
    }

    bool onebyone = !visibleChildren.isEmpty();
    // 排序
    if (!onebyone)
        Q_EMIT insertRows(0, newChildren.length());
    for (const auto &url : newChildren) {
        int showIndex = insertSortList(url, visibleChildren,
                                       AbstractSortFilter::SortScenarios::kSortScenariosIteratorExistingFile);
        if (isCanceled)
            return;
        if (onebyone)
            Q_EMIT insertRows(showIndex, 1);
        {
            QWriteLocker lk(&locker);
            visibleChildren.insert(showIndex, url);
        }
        if (onebyone)
            Q_EMIT insertFinish();
    }

    if (!onebyone)
        Q_EMIT insertFinish();

    if (isFinished) {
        Q_EMIT requestSetIdel();
    } else {
        Q_EMIT getSourceData(currentKey);
    }
}

void FileSortWorker::handleIteratorChild(const QString &key, const SortInfoPointer child, const FileInfoPointer info)
{
    if (isCanceled)
        return;
    if (currentKey != key)
        return;
    if (!child)
        return;

    addChild(child, info);
}

void FileSortWorker::handleIteratorChildren(const QString &key, QList<SortInfoPointer> children, QList<FileInfoPointer> infos)
{
    if (isCanceled)
        return;
    if (currentKey != key)
        return;

    int total = children.length();

    int showIndex = visibleChildren.length();

    QList<QUrl> listshow;

    for (int i = 0; i < total; ++i) {

        if (isCanceled)
            return;

        const auto &sortInfo = children.at(i);
        if (!sortInfo)
            continue;

        if (childrenUrlList.contains(sortInfo->fileUrl()))
            continue;

        this->children.append(sortInfo);
        childrenUrlList.append(sortInfo->fileUrl());
        {
            QWriteLocker lk(&childrenDataLocker);
            childrenDataMap.insert(sortInfo->fileUrl(), new FileItemData(sortInfo->fileUrl(), infos.at(i), rootdata));
        }
        if (!checkFilters(sortInfo))
            continue;

        listshow.append(sortInfo->fileUrl());
    }

    if (listshow.length() <= 0)
        return;

    Q_EMIT insertRows(showIndex, listshow.length());
    {
        QWriteLocker lk(&locker);
        visibleChildren.append(listshow);
    }
    Q_EMIT insertFinish();
}

void FileSortWorker::handleModelGetSourceData()
{
    if (isCanceled)
        return;
    emit getSourceData(currentKey);
}

void FileSortWorker::setFilters(QDir::Filters filters)
{
    resetFilters(filters);
}

void FileSortWorker::setNameFilters(const QStringList &filters)
{
    nameFilters = filters;
    QMap<QUrl, FileItemData *>::iterator itr = childrenDataMap.begin();
    for (; itr != childrenDataMap.end(); ++itr) {
        checkNameFilters(itr.value());
    }
    Q_EMIT requestUpdateView();
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

    setFilters(newFilters);
}

void FileSortWorker::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (isCanceled)
        return;

    if (aa == Application::kFileAndDirMixedSort)
        resort(sortOrder, orgSortRole, value.toBool());
}

void FileSortWorker::handleWatcherAddChildren(QList<SortInfoPointer> children)
{
    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;
        if (this->childrenUrlList.contains(sortInfo->fileUrl()))
            continue;
        addChild(sortInfo, AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile);
    }
}

void FileSortWorker::handleWatcherRemoveChildren(QList<SortInfoPointer> children)
{
    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;

        if (!sortInfo || !childrenUrlList.contains(sortInfo->fileUrl()))
            continue;

        auto index = childrenUrlList.indexOf(sortInfo->fileUrl());
        {
            QWriteLocker lk(&childrenDataLocker);
            childrenDataMap.remove(childrenUrlList.takeAt(index));
        }
        this->children.removeAt(index);

        int showIndex = -1;
        {
            QReadLocker lk(&locker);
            if (!visibleChildren.contains(sortInfo->fileUrl()))
                continue;
            showIndex = visibleChildren.indexOf(sortInfo->fileUrl());
            if (showIndex <= -1)
                continue;
        }

        Q_EMIT removeRows(showIndex, 1);
        {
            QWriteLocker lk(&locker);
            visibleChildren.removeAt(showIndex);
        }
        Q_EMIT removeFinish();
    }
}

void FileSortWorker::resort(const Qt::SortOrder order, const ItemRoles sortRole, const bool isMixDirAndFile)
{
    if (isCanceled)
        return;

    auto opt = setSortAgruments(order, sortRole, isMixDirAndFile);
    switch (opt) {
    case FileSortWorker::SortOpt::kSortOptOtherChanged:
        return sortAllFiles();
    case FileSortWorker::SortOpt::kSortOptOnlyOrderChanged:
        return sortOnlyOrderChange();
    default:
        return;
    }
}

void FileSortWorker::handleTraversalFinish(const QString &key)
{
    if (currentKey != key)
        return;

    Q_EMIT requestSetIdel();

    setNameFilters(nameFilters);
}

void FileSortWorker::handleSortAll(const QString &key)
{
    if (currentKey != key)
        return;
    sortAllFiles();
}

void FileSortWorker::handleWatcherUpdateFile(const SortInfoPointer child)
{
    if (isCanceled)
        return;

    if (!child)
        return;

    if (!child->fileUrl().isValid() || !childrenUrlList.contains(child->fileUrl()))
        return;

    FileInfoPointer info;
    {
        QReadLocker lk(&childrenDataLocker);
        info = childrenDataMap.value(child->fileUrl())->fileInfo();
    }

    if (!info)
        return;

    info->refresh();

    sortInfoUpdateByFileInfo(info);

    handleUpdateFile(child->fileUrl());
}

void FileSortWorker::handleWatcherUpdateHideFile(const QUrl &hidUrl)
{
    if (isCanceled)
        return;
    auto hiddenFileInfo = InfoFactory::create<FileInfo>(hidUrl);
    if (!hiddenFileInfo)
        return;
    auto hidlist = DFMUtils::hideListFromUrl(QUrl::fromLocalFile(hiddenFileInfo->pathOf(PathInfoType::kFilePath)));
    for (const auto &child : children) {
        if (isCanceled)
            return;

        auto info = InfoFactory::create<FileInfo>(child->fileUrl());
        if (!info)
            continue;
        auto fileName = info->nameOf(NameInfoType::kFileName);
        if (fileName.startsWith(".")) {
            child->setHide(true);
        } else {
            child->setHide(hidlist.contains(fileName));
        }
        info->setExtendedAttributes(ExtInfoType::kFileIsHid, child->isHide());
    }

    filterAllFiles(true);
}

void FileSortWorker::handleUpdateFile(const QUrl &url)
{
    if (isCanceled)
        return;

    if (!url.isValid() || !childrenUrlList.contains(url))
        return;

    SortInfoPointer sortInfo = children.at(childrenUrlList.indexOf(url));
    if (!sortInfo)
        return;

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
            return;
        }
        Q_EMIT updateRow(childIndex);
        return;
    }

    if (checkFilters(sortInfo, true)) {
        int showIndex = visibleChildren.length();
        // kItemDisplayRole 是不进行排序的
        if (orgSortRole != Global::ItemRoles::kItemDisplayRole)
            showIndex = insertSortList(sortInfo->fileUrl(), visibleChildren, AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile);

        if (isCanceled)
            return;

        Q_EMIT insertRows(showIndex, 1);
        {
            QWriteLocker lk(&locker);
            visibleChildren.insert(showIndex, sortInfo->fileUrl());
        }
        Q_EMIT insertFinish();

        // async create file will add to view while file info updated.
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());
    }
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

void FileSortWorker::handleRefresh()
{
    bool empty { false };
    {
        QReadLocker lk(&locker);
        empty = visibleChildren.isEmpty();
    }

    if (!empty)
        Q_EMIT removeRows(0, visibleChildren.length());

    {
        QWriteLocker lk(&locker);
        visibleChildren.clear();
        children.clear();
    }

    {
        QWriteLocker lk(&childrenDataLocker);
        childrenUrlList.clear();
        qDeleteAll(childrenDataMap.values());
        childrenDataMap.clear();
    }

    if (!empty)
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
    if (!childrenUrlList.contains(url))
        return;

    auto itemdata = childData(url);
    if (!itemdata)
        return;

    auto fileInfo = itemdata->fileInfo();
    if (!fileInfo || QString::number(quintptr(fileInfo.data()), 16) != infoPtr)
        return;

    fileInfo->customData(Global::ItemRoles::kItemFileRefreshIcon);

    sortInfoUpdateByFileInfo(fileInfo);

    handleUpdateFile(url);
}

void FileSortWorker::checkNameFilters(FileItemData *itemData)
{
    if (!itemData || itemData->data(Global::ItemRoles::kItemFileIsDirRole).toBool() || nameFilters.isEmpty())
        return;

    QRegExp re("", Qt::CaseInsensitive, QRegExp::Wildcard);
    for (int i = 0; i < nameFilters.size(); ++i) {
        re.setPattern(nameFilters.at(i));
        if (re.exactMatch(itemData->data(kItemNameRole).toString())) {
            itemData->setAvailableState(true);
            return;
        }
    }

    itemData->setAvailableState(false);
}

bool FileSortWorker::checkFilters(const SortInfoPointer &sortInfo, const bool byInfo)
{
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

void FileSortWorker::filterAllFiles(const bool byInfo)
{
    QList<QUrl> filterUrls {};
    for (const auto &sortInfo : children) {
        if (checkFilters(sortInfo, byInfo))
            filterUrls.append(sortInfo->fileUrl());
        if (isCanceled)
            return;
    }

    if (filterUrls.isEmpty()) {
        int count = childrenCount();
        if (count > 0) {
            Q_EMIT removeRows(0, count);
            {
                QWriteLocker lk(&locker);
                visibleChildren.clear();
            }
            Q_EMIT removeFinish();
        }
        return;
    }

    Q_EMIT insertRows(0, filterUrls.length());
    {
        QWriteLocker lk(&locker);
        visibleChildren = filterUrls;
    }
    Q_EMIT insertFinish();
}

void FileSortWorker::filterAllFilesOrdered()
{
    for (const auto &sortInfo : children) {
        if (isCanceled)
            return;
        auto index = -1;
        index = visibleChildren.indexOf(sortInfo->fileUrl());

        bool show = checkFilters(sortInfo, true);
        // 显示出来，显示中包含或者 不现实出来，显示中不包含 跳过
        if (show ^ (index < 0))
            continue;
        if (show) {
            auto showIndex = insertSortList(sortInfo->fileUrl(), visibleChildren,
                                            AbstractSortFilter::SortScenarios::kSortScenariosWatcherOther);
            Q_EMIT insertRows(showIndex, 1);
            {
                QWriteLocker lk(&locker);
                visibleChildren.insert(showIndex, sortInfo->fileUrl());
            }
            Q_EMIT insertFinish();
        } else {
            Q_EMIT removeRows(index, 1);
            {
                QWriteLocker lk(&locker);
                visibleChildren.removeAt(index);
            }
            Q_EMIT removeFinish();
        }
    }
}

void FileSortWorker::sortAllFiles()
{
    if (isCanceled)
        return;

    if (orgSortRole == Global::ItemRoles::kItemDisplayRole)
        return;

    if (visibleChildren.count() <= 1)
        return;

    QList<QUrl> sortList;
    int i = 0;
    bool sortSame = true;
    for (const auto &url : visibleChildren) {
        if (isCanceled)
            return;
        auto sortIndex = insertSortList(url, sortList, AbstractSortFilter::SortScenarios::kSortScenariosNormal);
        if (sortSame)
            sortSame = sortIndex == i;

        sortList.insert(sortIndex, url);
        i++;
    }

    if (sortSame)
        return;

    Q_EMIT insertRows(0, sortList.length());
    {
        QWriteLocker lk(&locker);
        visibleChildren = sortList;
    }
    Q_EMIT insertFinish();
}

void FileSortWorker::sortOnlyOrderChange()
{
    if (isCanceled)
        return;

    if (orgSortRole == Global::ItemRoles::kItemDisplayRole)
        return;

    if (isMixDirAndFile) {
        QList<QUrl> sortList;
        for (const auto &url : visibleChildren) {
            sortList.push_front(url);
        }
        Q_EMIT insertRows(0, sortList.length());
        {
            QWriteLocker lk(&locker);
            visibleChildren = sortList;
        }
        Q_EMIT insertFinish();
        return;
    }

    QList<QUrl> dirList, fileList;
    for (const auto &url : visibleChildren) {
        const auto &info = InfoFactory::create<FileInfo>(url);
        if (!info)
            continue;
        if (info->isAttributes(OptInfoType::kIsDir)) {
            dirList.push_front(url);
        } else {
            fileList.push_front(url);
        }
    }
    dirList.append(fileList);
    Q_EMIT insertRows(0, dirList.length());
    {
        QWriteLocker lk(&locker);
        visibleChildren = dirList;
    }
    Q_EMIT insertFinish();
    return;
}

void FileSortWorker::addChild(const SortInfoPointer &sortInfo, const FileInfoPointer &info)
{
    if (isCanceled)
        return;

    if (!sortInfo)
        return;

    if (childrenUrlList.contains(sortInfo->fileUrl()))
        return;

    children.append(sortInfo);
    childrenUrlList.append(sortInfo->fileUrl());
    {
        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.insert(sortInfo->fileUrl(), new FileItemData(sortInfo->fileUrl(), info, rootdata));
    }
    if (!checkFilters(sortInfo))
        return;

    if (isCanceled)
        return;
    int showIndex = visibleChildren.length();

    if (isCanceled)
        return;

    Q_EMIT insertRows(showIndex, 1);
    {
        QWriteLocker lk(&locker);
        visibleChildren.append(sortInfo->fileUrl());
    }
    Q_EMIT insertFinish();
}

void FileSortWorker::addChild(const SortInfoPointer &sortInfo,
                              const AbstractSortFilter::SortScenarios sort)
{
    if (isCanceled)
        return;

    if (!sortInfo)
        return;

    if (childrenUrlList.contains(sortInfo->fileUrl()))
        return;

    children.append(sortInfo);
    childrenUrlList.append(sortInfo->fileUrl());
    {
        auto info = InfoFactory::create<FileInfo>(sortInfo->fileUrl());
        FileItemData *item{nullptr};
        if (info) {
            info->refresh();
            item = new FileItemData(sortInfo->fileUrl(), info, rootdata);
            item->setSortFileInfo(sortInfo);
        } else {
            item = new FileItemData(sortInfo, rootdata);
        }

        QWriteLocker lk(&childrenDataLocker);
        childrenDataMap.insert(sortInfo->fileUrl(), item);
    }

    if (!checkFilters(sortInfo, true))
        return;

    if (isCanceled)
        return;

    int showIndex = visibleChildren.length();
    // kItemDisplayRole 是不进行排序的
    if (orgSortRole != Global::ItemRoles::kItemDisplayRole)
        showIndex = insertSortList(sortInfo->fileUrl(), visibleChildren, sort);

    if (isCanceled)
        return;

    Q_EMIT insertRows(showIndex, 1);
    {
        QWriteLocker lk(&locker);
        visibleChildren.insert(showIndex, sortInfo->fileUrl());
    }
    Q_EMIT insertFinish();

    if (sort == AbstractSortFilter::SortScenarios::kSortScenariosWatcherAddFile)
        Q_EMIT selectAndEditFile(sortInfo->fileUrl());
}

bool FileSortWorker::sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo)
{
    if (!fileInfo)
        return false;

    auto url = fileInfo->fileUrl();
    if (!childrenUrlList.contains(url))
        return false;

    int index = childrenUrlList.indexOf(url);
    if (index < 0 || children.count() <= index)
        return false;

    SortInfoPointer sortInfo = children.at(index);
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
