// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "filesortworker.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <QStandardPaths>

using namespace dfmplugin_workspace;
using namespace dfmbase;
using namespace dfmbase::Global;
using namespace dfmio;

FileSortWorker::FileSortWorker(const QUrl &url, const QStringList &nameFilters, const QDir::Filters filters, const QDirIterator::IteratorFlags flags, QObject *parent)
    : QObject(parent), current(url), nameFilters(nameFilters), filters(filters), flags(flags)
{

}

void FileSortWorker::setSortAgruments(const Qt::SortOrder order, const Global::ItemRoles sortRole)
{
    sortOrder = order;
    this->isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    orgSortRole = sortRole;
    switch (sortRole) {
    case Global::ItemRoles::kItemFileDisplayNameRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName;
        break;
    case Global::ItemRoles::kItemFileSizeRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileSize;
        break;
    case Global::ItemRoles::kItemFileLastReadRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastRead;
        break;
    case Global::ItemRoles::kItemFileLastModifiedRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastModified;
        break;
    default:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
    }
}

int FileSortWorker::mapToIndex(int index)
{
    QReadLocker lk(&locker);
    if (index < 0 || index >= visibleChildrenIndex.count())
        return -1;
    return visibleChildrenIndex.at(index);
}

int FileSortWorker::childrenCount()
{
    QReadLocker lk(&locker);
    return visibleChildrenIndex.count();
}

void FileSortWorker::updateSortChildren(QList<QSharedPointer<dfmio::DEnumerator::SortFileInfo> > children,
                                        dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                        Qt::SortOrder sortOrder,
                                        bool isMixDirAndFile)
{
    this->isMixDirAndFile = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    this->children = children;
    filterAllFiles();
    if (this->sortRole == sortRole && this->sortOrder == sortOrder && this->isMixDirAndFile == isMixDirAndFile) {
        emit sortAllFiles();
        return;
    }

    emit allDataReady();

    sortAllFiles();

    emit allDataSortedCompleted();
}

void FileSortWorker::updateChild(const AbstractFileInfoPointer child)
{
    QSharedPointer<dfmio::DEnumerator::SortFileInfo> sortInfo(new dfmio::DEnumerator::SortFileInfo);
    sortInfo->url = child->urlOf(UrlInfoType::kUrl);
    sortInfo->isDir = child->isAttributes(OptInfoType::kIsDir);
    sortInfo->isFile = child->isAttributes(OptInfoType::kIsFile);
    sortInfo->isHide = child->isAttributes(OptInfoType::kIsHidden);
    sortInfo->isSymLink = child->isAttributes(OptInfoType::kIsSymLink);
    sortInfo->isReadable = child->isAttributes(OptInfoType::kIsReadable);
    sortInfo->isWriteable = child->isAttributes(OptInfoType::kIsWritable);
    sortInfo->isExecutable = child->isAttributes(OptInfoType::kIsExecutable);
    updateChild(sortInfo);
}

void FileSortWorker::getDataAndUpdate()
{
    // 使用url去数据区获取全部文件
    QList<QSharedPointer<dfmio::DEnumerator::SortFileInfo>> children;
    // 获取相对于已有的新增加的文件
    QList<int> newChildren;
    int start = this->children.count();
    for (const auto &sortInfo :  children) {
        if (this->children.contains(sortInfo))
            continue;
        this->children.append(sortInfo);
        if (checkFilters(sortInfo))
            newChildren.append(start);
        start++;
    }
    // 排序
    for (const auto index : newChildren) {
        int showIndex = insertSortList(index, visibleChildrenIndex);
        QWriteLocker lk(&locker);
        visibleChildrenIndex.insert(showIndex,index);
    }
    // 通知主界面全部刷新
    emit allDataSortedCompleted();
}

void FileSortWorker::resetFilters(const QStringList &nameFilters, const QDir::Filters filters)
{
    this->nameFilters = nameFilters;
    this->filters = filters;
    // 通过自己的url去数据区获取所有数据
    QList<QSharedPointer<dfmio::DEnumerator::SortFileInfo>> datas;
    children = datas;
    filterAllFiles();
    sortAllFiles();
    emit allDataSortedCompleted();
}

void FileSortWorker::addChild(const QSharedPointer<DEnumerator::SortFileInfo> &sortInfo)
{
    if (children.contains(sortInfo))
        return;
    updateChild(sortInfo);
}

void FileSortWorker::removeChild(const QSharedPointer<DEnumerator::SortFileInfo> &sortInfo)
{
    auto index = children.indexOf(sortInfo);
    int showIndex = -1;
    {
        QReadLocker lk(&locker);
        if (index < 0 || !visibleChildrenIndex.contains(index))
            return;
        showIndex = visibleChildrenIndex.indexOf(index);
    }

    if (showIndex > -1)
        emit removeFileCompleted(showIndex);
}

void FileSortWorker::resort(const Qt::SortOrder order, const ItemRoles sortRole)
{
    setSortAgruments(order, sortRole);
    sortAllFiles();
}

bool FileSortWorker::checkFilters(const QSharedPointer<dfmio::DEnumerator::SortFileInfo> &sortInfo)
{
    if (filters == QDir::NoFilter)
        return true;

    const bool isDir = sortInfo->isDir;
    if ((filters & QDir::AllDirs) == QDir::AllDirs) {
        // all dir, no apply filters rules
        if (isDir)
            return true;
    }

    // dir filter
    const bool readable = sortInfo->isReadable;
    const bool writable = sortInfo->isWriteable;
    const bool executable = sortInfo->isExecutable;

    auto checkRWE = [&]() -> bool {
        if ((filters & QDir::Readable) == QDir::Readable)
        {
            if (!readable)
                return false;
        }
        if ((filters & QDir::Writable) == QDir::Writable)
        {
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
        const bool isFile = sortInfo->isFile;
        if (!isFile) {
            return false;
        } else {
            // 判断读写执行
            if (!checkRWE())
                return false;
        }
    }

    if ((filters & QDir::NoSymLinks) == QDir::NoSymLinks)
    {
        const bool isSymlinks = sortInfo->isSymLink;
        if (isSymlinks)
            return false;
    }

    const bool showHidden = (filters & QDir::Hidden) == QDir::Hidden;
    if (!showHidden) {   // hide files
        bool isHidden = sortInfo->isHide;
        if (isHidden)
            return false;
    }

    if (nameFilters.isEmpty())
        return true;

    QString path(sortInfo->url.path());
    const QString &fileInfoName = path.right(path.lastIndexOf("/"));
    // filter name
    const bool caseSensitive = (filters & QDir::CaseSensitive) == QDir::CaseSensitive;
    if (nameFilters.contains(fileInfoName, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
        return false;

    return true;
}

void FileSortWorker::filterAllFiles()
{
    int i = 0;
    for (const auto &sortInfo : children) {
        if (checkFilters(sortInfo))
            visibleChildrenIndex.append(i);
        i++;
    }
}

void FileSortWorker::sortAllFiles()
{
    QList<int> sortList;
    for (const auto index : visibleChildrenIndex) {
        sortList.insert(insertSortList(index, sortList),index);
    }
    QWriteLocker lk(&locker);
    visibleChildrenIndex = sortList;
}

void FileSortWorker::updateChild(const QSharedPointer<DEnumerator::SortFileInfo> &sortInfo)
{
    children.append(sortInfo);
    if (!checkFilters(sortInfo))
        return;

    int index = children.count() - 1;
    int showIndex = insertSortList(index, visibleChildrenIndex);
    {
        QWriteLocker lk(&locker);
        visibleChildrenIndex.insert(showIndex,index);
    }
    // 通知主界面
    emit addFileCompleted(index);
}

bool FileSortWorker::lessThan(const int left, const int right)
{
    int childCount = children.count();
    if (left >= childCount)
        return false;
    if (right >= childCount)
        return false;

    const AbstractFileInfoPointer &leftInfo = InfoFactory::create<AbstractFileInfo>(children[left]->url);
    const AbstractFileInfoPointer &rightInfo = InfoFactory::create<AbstractFileInfo>(children[right]->url);

    if (!leftInfo)
        return false;
    if (!rightInfo)
        return false;

    // The folder is fixed in the front position
    if (!isMixDirAndFile) {
        if (leftInfo->isAttributes(OptInfoType::kIsDir)) {
            if (!rightInfo->isAttributes(OptInfoType::kIsDir))
                return sortOrder == Qt::AscendingOrder;
        } else {
            if (rightInfo->isAttributes(OptInfoType::kIsDir))
                return sortOrder == Qt::DescendingOrder;
        }
    }

    QVariant leftData = data(leftInfo, orgSortRole);
    QVariant rightData = data(rightInfo, orgSortRole);

    // When the selected sort attribute value is the same, sort by file name
    if (leftData == rightData) {
        QString leftName = leftInfo->displayOf(DisPlayInfoType::kFileDisplayName);
        QString rightName = rightInfo->displayOf(DisPlayInfoType::kFileDisplayName);
        return FileUtils::compareString(leftName, rightName, sortOrder);
    }

    switch (orgSortRole) {
    case kItemFileDisplayNameRole:
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder) == (sortOrder == Qt::AscendingOrder);
    case kItemFileSizeRole:
        if (!isMixDirAndFile) {
            if (leftInfo->isAttributes(OptInfoType::kIsDir)) {
                int leftCount = leftInfo->countChildFile();
                int rightCount = rightInfo->countChildFile();
                return leftCount < rightCount;
            } else {
                return leftInfo->size() < rightInfo->size();
            }
        } else {
            qint64 sizel = leftInfo->isAttributes(OptInfoType::kIsDir) && rightInfo->isAttributes(OptInfoType::kIsDir)
                    ? leftInfo->countChildFile()
                    : (leftInfo->isAttributes(OptInfoType::kIsDir) ? 0 : leftInfo->size());
            qint64 sizer = leftInfo->isAttributes(OptInfoType::kIsDir) && rightInfo->isAttributes(OptInfoType::kIsDir)
                    ? rightInfo->countChildFile()
                    : (rightInfo->isAttributes(OptInfoType::kIsDir) ? 0 : rightInfo->size());
            return sizel < sizer;
        }
    default:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder) == (sortOrder == Qt::AscendingOrder);
    }
}

QVariant FileSortWorker::data(const AbstractFileInfoPointer &info, ItemRoles role)
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

int FileSortWorker::insertSortList(const int needNode, const QList<int> &list)
{
    int begin = 0;
    int end = list.count();
    int row = (begin + end)/2;
    // 先找到文件还是目录
    forever {
        if (isCanceled)
            return row;

        if (begin == end)
            break;

        const int node = list.at(row);
        if (!lessThan(needNode, node)) {
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
