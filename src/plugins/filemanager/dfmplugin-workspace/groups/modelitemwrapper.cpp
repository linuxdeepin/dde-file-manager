// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modelitemwrapper.h"
#include "filegroupdata.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

ModelItemWrapper::ModelItemWrapper()
    : itemType(FileItem)
{
}

ModelItemWrapper::ModelItemWrapper(const FileItemDataPointer &fileData, const QString &groupKey)
    : itemType(FileItem), groupKey(groupKey), fileData(fileData)
{
}

ModelItemWrapper::ModelItemWrapper(const FileGroupData *groupData)
    : itemType(GroupHeaderItem), groupKey(groupData ? groupData->groupKey : QString())
{
    if (groupData) {
        // Use special URL format to identify group headers
        QUrl groupHeaderUrl = QUrl::fromUserInput(QString("group-header://%1").arg(groupData->groupKey));

        // Create a special FileItemData to represent group headers
        // Pass nullptr as FileInfoPointer to mark it as a special group header

        FileItemDataPointer headerData = QSharedPointer<FileItemData>::create(groupHeaderUrl);
        fileData = headerData;

        // Store group values directly instead of pointer
        groupValues[Global::kItemIsGroupHeaderType] = true;
        groupValues[Global::kItemDisplayRole] = groupData->getHeaderText();
        groupValues[Global::kItemNameRole] = groupData->getHeaderText();
        groupValues[Global::kItemGroupFileCount] = groupData->fileCount;
        groupValues[Global::kItemGroupHeaderKey] = groupData->groupKey;
        groupValues[Global::kItemGroupDisplayIndex] = groupData->displayIndex;
        groupValues[Global::kItemGroupExpandedRole] = groupData->isExpanded;
    }
}

ModelItemWrapper::ModelItemWrapper(const ModelItemWrapper &other)
    : itemType(other.itemType), groupKey(other.groupKey), fileData(other.fileData), groupValues(other.groupValues)
{
}

ModelItemWrapper &ModelItemWrapper::operator=(const ModelItemWrapper &other)
{
    if (this != &other) {
        itemType = other.itemType;
        groupKey = other.groupKey;
        fileData = other.fileData;
        groupValues = other.groupValues;
    }
    return *this;
}

ModelItemWrapper::~ModelItemWrapper()
{
    // No manual memory management needed
}

bool ModelItemWrapper::isGroupHeader() const
{
    return itemType == GroupHeaderItem;
}

bool ModelItemWrapper::isFileItem() const
{
    return itemType == FileItem;
}

QVariant ModelItemWrapper::getData(int role) const
{
    if (itemType == FileItem && fileData) {
        switch (role) {
        case Global::kItemIsGroupHeaderType:
            return false;
        }
        // For file items, delegate to the file data
        return fileData->data(role);
    } else if (itemType == GroupHeaderItem) {
        // For group headers, provide appropriate data from groupValues
        auto it = groupValues.find(role);
        if (it != groupValues.end()) {
            return it.value();
        }
    }

    return QVariant();
}

bool ModelItemWrapper::isValid() const
{
    if (itemType == FileItem) {
        return fileData != nullptr;
    } else if (itemType == GroupHeaderItem) {
        return !groupValues.isEmpty();
    }

    return false;
}

DPWORKSPACE_END_NAMESPACE
