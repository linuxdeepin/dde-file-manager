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

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileSortFilterProxyModel::FileSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

FileSortFilterProxyModel::~FileSortFilterProxyModel()
{
}

void FileSortFilterProxyModel::setFilterData(const QVariant &data)
{
    filterData = data;
}

void FileSortFilterProxyModel::setFilterCallBack(const FileViewFilterCallback callback)
{
    filterCallback = callback;
}

void FileSortFilterProxyModel::resetFilter()
{
    filterData = QVariant();
    filterCallback = nullptr;
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
        QString leftName = fileModel->data(left, FileViewItem::kItemNameRole).toString();
        QString rightName = fileModel->data(right, FileViewItem::kItemNameRole).toString();
        return QString::localeAwareCompare(leftName, rightName) < 0;
    }

    switch (sortRole()) {
    case FileViewItem::kItemNameRole:
    case FileViewItem::kItemFileLastModifiedRole:
    case FileViewItem::kItemFileMimeTypeRole:
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    case FileViewItem::kItemFileSizeRole:
        if (leftInfo->isDir()) {
            int leftCount = qSharedPointerDynamicCast<LocalFileInfo>(leftInfo)->countChildFile();
            int rightCount = qSharedPointerDynamicCast<LocalFileInfo>(rightInfo)->countChildFile();
            return leftCount < rightCount;
        } else {
            return leftInfo->size() < rightInfo->size();
        }
    default:
        return false;
    }
}

bool FileSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    FileViewModel *fileModel = qobject_cast<FileViewModel *>(sourceModel());
    QModelIndex rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    AbstractFileInfoPointer fileInfo = fileModel->itemFromIndex(rowIndex)->fileInfo();

    if (filterCallback)
        return filterCallback(fileInfo.data(), filterData);

    return fileInfo && !fileInfo->isHidden();
}
