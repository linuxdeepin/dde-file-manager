/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "filesortfilterproxymodel.h"
#include "fileviewmodel.h"

#include "base/application/settings.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/universalutils.h"

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileSortFilterProxyModel::FileSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    resetFilter();
}

FileSortFilterProxyModel::~FileSortFilterProxyModel()
{
}

int FileSortFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::rowCount(parent);
}

int FileSortFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // TODO():
    return getColumnRoles().length();
}

QVariant FileSortFilterProxyModel::headerData(int column, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        int column_role = getRoleByColumn(column);
        return roleDisplayString(column_role);
    }

    return QVariant();
}

bool FileSortFilterProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    bool isRootIndex = parent == rootIndex();
    const QModelIndex &sourceIndex = isRootIndex ? parent : mapToSource(parent);
    return viewModel()->dropMimeData(data, action, sourceIndex.row(), sourceIndex.column(), sourceIndex);
}

Qt::ItemFlags FileSortFilterProxyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);

    const QModelIndex &sourceIndex = mapToSource(index);
    if (sourceIndex.isValid()) {
        const FileViewItem *item = viewModel()->itemFromIndex(sourceIndex);
        if (item) {
            const AbstractFileInfoPointer info = item->fileInfo();
            if (!passNameFilters(info))
                flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }
    }

    if (readOnly)
        flags &= ~(Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);

    return flags;
}

QModelIndex FileSortFilterProxyModel::setRootUrl(const QUrl &url)
{
    QModelIndex rootIndex = viewModel()->setRootUrl(url);
    resetFilter();

    return createIndex(0, 0, rootIndex.internalPointer());
}

QUrl FileSortFilterProxyModel::rootUrl() const
{
    return viewModel()->rootUrl();
}

QModelIndex FileSortFilterProxyModel::rootIndex() const
{
    const QModelIndex &sourceRootIndex = viewModel()->rootIndex();
    return createIndex(sourceRootIndex.row(), sourceRootIndex.column(), sourceRootIndex.internalPointer());
}

const FileViewItem *FileSortFilterProxyModel::rootItem() const
{
    return viewModel()->rootItem();
}

const FileViewItem *FileSortFilterProxyModel::itemFromIndex(const QModelIndex &index) const
{
    const QModelIndex &sourceIndex = mapToSource(index);
    return viewModel()->itemFromIndex(sourceIndex);
}

AbstractFileInfoPointer FileSortFilterProxyModel::itemFileInfo(const QModelIndex &index) const
{
    const QModelIndex &sourceIndex = mapToSource(index);
    return viewModel()->fileInfo(sourceIndex);
}

QModelIndex FileSortFilterProxyModel::getIndexByUrl(const QUrl &url) const
{
    FileViewModel *fileModel = qobject_cast<FileViewModel *>(sourceModel());
    QModelIndex sourceIndex = fileModel->findIndex(url);

    if (sourceIndex.isValid()) {
        auto tempIndex = mapFromSource(sourceIndex);

        // Importent: The mapping object will not update when the row count changed,
        // therefore can not map source to proxy. So traverse the model to find the proxy index.
        if (!tempIndex.isValid()) {
            for (int i = 0; i < rowCount(); ++i) {
                sourceIndex = mapToSource(index(i, 0));
                if (sourceIndex.isValid() && UniversalUtils::urlEquals(sourceIndex.data(kItemUrlRole).toUrl(), url))
                    return index(i, 0);
            }
        }
        return index(tempIndex.row(), tempIndex.column());
    }

    return QModelIndex();
}

QUrl FileSortFilterProxyModel::getUrlByIndex(const QModelIndex &index) const
{
    const QModelIndex &sourceIndex = mapToSource(index);
    return sourceIndex.data(kItemUrlRole).toUrl();
}

QList<QUrl> FileSortFilterProxyModel::getCurrentDirFileUrls() const
{
    QList<QUrl> urls {};
    int count = rowCount();
    for (int i = 0; i < count; ++i) {
        const QModelIndex &proxyIndex = index(i, 0);
        urls << getUrlByIndex(proxyIndex);
    }

    return urls;
}

int FileSortFilterProxyModel::getColumnWidth(const int &column) const
{
    const ItemRoles role = getRoleByColumn(column);

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();
    int colWidth = state.value(QString::number(role), -1).toInt();
    if (colWidth > 0) {
        return colWidth;
    }

    return 120;
}

ItemRoles FileSortFilterProxyModel::getRoleByColumn(const int &column) const
{
    QList<ItemRoles> columnRoleList = getColumnRoles();

    if (columnRoleList.length() > column)
        return columnRoleList.at(column);

    return kItemNameRole;
}

int FileSortFilterProxyModel::getColumnByRole(const ItemRoles role) const
{
    QList<ItemRoles> columnRoleList = getColumnRoles();
    return columnRoleList.indexOf(role) < 0 ? 0 : columnRoleList.indexOf(role);
}

QDir::Filters FileSortFilterProxyModel::getFilters() const
{
    return filters;
}

void FileSortFilterProxyModel::setFilters(const QDir::Filters &filters)
{
    this->filters = filters;

    invalidateFilter();
}

void FileSortFilterProxyModel::setNameFilters(const QStringList &nameFilters)
{
    if (this->nameFilters == nameFilters) {
        return;
    }

    nameFiltersMatchResultMap.clear();
    this->nameFilters = nameFilters;

    invalidateFilter();
}

void FileSortFilterProxyModel::setFilterData(const QVariant &data)
{
    filterData = data;
    invalidateFilter();
}

void FileSortFilterProxyModel::setFilterCallBack(const FileViewFilterCallback callback)
{
    filterCallback = callback;
    invalidateFilter();
}

void FileSortFilterProxyModel::resetFilter()
{
    filterData = QVariant();
    filterCallback = nullptr;
    filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    bool isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    if (isShowedHiddenFiles) {
        filters |= QDir::Hidden;
    } else {
        filters &= ~QDir::Hidden;
    }
    invalidateFilter();
}

void FileSortFilterProxyModel::toggleHiddenFiles()
{
    filters = ~(filters ^ QDir::Filter(~QDir::Hidden));
    setFilters(filters);
}

void FileSortFilterProxyModel::setReadOnly(const bool readOnly)
{
    this->readOnly = readOnly;
}

bool FileSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!left.isValid())
        return false;
    if (!right.isValid())
        return false;

    FileViewModel *fileModel = qobject_cast<FileViewModel *>(sourceModel());

    const FileViewItem *leftItem = fileModel->itemFromIndex(left);
    const FileViewItem *rightItem = fileModel->itemFromIndex(right);

    if (!leftItem)
        return false;
    if (!rightItem)
        return false;

    AbstractFileInfoPointer leftInfo = leftItem->fileInfo();
    AbstractFileInfoPointer rightInfo = rightItem->fileInfo();

    if (!leftInfo)
        return false;
    if (!rightInfo)
        return false;

    // The folder is fixed in the front position
    if (leftInfo->isDir()) {
        if (!rightInfo->isDir())
            return sortOrder() == Qt::AscendingOrder;
    } else {
        if (rightInfo->isDir())
            return sortOrder() == Qt::DescendingOrder;
    }

    QVariant leftData = fileModel->data(left, sortRole());
    QVariant rightData = fileModel->data(right, sortRole());

    // When the selected sort attribute value is the same, sort by file name
    if (leftData == rightData) {
        QString leftName = fileModel->data(left, kItemNameRole).toString();
        QString rightName = fileModel->data(right, kItemNameRole).toString();
        return FileUtils::compareString(leftName, rightName, sortOrder());
    }

    switch (sortRole()) {
    case kItemNameRole:
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder()) == (sortOrder() == Qt::AscendingOrder);
    case kItemFileSizeRole:
        if (leftInfo->isDir()) {
            int leftCount = qSharedPointerDynamicCast<AbstractFileInfo>(leftInfo)->countChildFile();
            int rightCount = qSharedPointerDynamicCast<AbstractFileInfo>(rightInfo)->countChildFile();
            return leftCount < rightCount;
        } else {
            return leftInfo->size() < rightInfo->size();
        }
    default:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder()) == (sortOrder() == Qt::AscendingOrder);
    }
}

bool FileSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    FileViewModel *fileModel = qobject_cast<FileViewModel *>(sourceModel());
    QModelIndex rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!rowIndex.isValid())
        return false;

    AbstractFileInfoPointer fileInfo = fileModel->itemFromIndex(rowIndex)->fileInfo();

    return passFileFilters(fileInfo);
}

bool FileSortFilterProxyModel::passFileFilters(const AbstractFileInfoPointer &fileInfo) const
{
    if (!fileInfo)
        return false;

    if (filterCallback && !filterCallback(fileInfo.data(), filterData))
        return false;

    if (filters == QDir::NoFilter)
        return true;

    if (!(filters & (QDir::Dirs | QDir::AllDirs)) && fileInfo->isDir())
        return false;

    if (!(filters & QDir::Files) && fileInfo->isFile())
        return false;

    if ((filters & QDir::NoSymLinks) && fileInfo->isSymLink())
        return false;

    if (!(filters & QDir::Hidden) && fileInfo->isHidden())
        return false;

    if ((filters & QDir::Readable) && !fileInfo->isReadable())
        return false;

    if ((filters & QDir::Writable) && !fileInfo->isWritable())
        return false;

    if ((filters & QDir::Executable) && !fileInfo->isExecutable())
        return false;

    //Todo(yanghao):
    return true;
}

bool FileSortFilterProxyModel::passNameFilters(const AbstractFileInfoPointer &info) const
{
    if (nameFilters.isEmpty())
        return true;

    if (!info)
        return true;

    const QUrl fileUrl = info->url();
    if (nameFiltersMatchResultMap.contains(fileUrl))
        return nameFiltersMatchResultMap.value(fileUrl, false);

    // Check the name regularexpression filters
    if (!(info->isDir() && (filters & QDir::Dirs))) {
        const Qt::CaseSensitivity caseSensitive = (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const QString &fileDisplayName = info->fileDisplayName();
        QRegExp re("", caseSensitive, QRegExp::Wildcard);

        for (int i = 0; i < nameFilters.size(); ++i) {
            re.setPattern(nameFilters.at(i));
            if (re.exactMatch(fileDisplayName)) {
                nameFiltersMatchResultMap[fileUrl] = true;
                return true;
            }
        }

        nameFiltersMatchResultMap[fileUrl] = false;
        return false;
    }

    nameFiltersMatchResultMap[fileUrl] = true;
    return true;
}

FileViewModel *FileSortFilterProxyModel::viewModel() const
{
    return qobject_cast<FileViewModel *>(sourceModel());
}

QString FileSortFilterProxyModel::roleDisplayString(int role) const
{
    QString displayName;

    if (WorkspaceEventSequence::instance()->doFetchCustomRoleDiaplayName(rootUrl(), static_cast<ItemRoles>(role), &displayName))
        return displayName;

    switch (role) {
    case kItemNameRole:
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

QList<ItemRoles> FileSortFilterProxyModel::getColumnRoles() const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(rootUrl(), &roles);

    const QVariantMap &map = DFMBASE_NAMESPACE::Application::appObtuselySetting()->value("FileViewState", rootUrl()).toMap();
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
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemNameRole
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
