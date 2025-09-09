// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modelitemwrapper.h"
#include "filegroupdata.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

ModelItemWrapper::ModelItemWrapper()
    : itemType(FileItem), groupData(nullptr)
{
}

ModelItemWrapper::ModelItemWrapper(const FileItemDataPointer &fileData, const QString &groupKey)
    : itemType(FileItem), groupKey(groupKey), fileData(fileData), groupData(nullptr)
{
}

ModelItemWrapper::ModelItemWrapper(const FileGroupData *groupData)
    : itemType(GroupHeaderItem), groupKey(groupData ? groupData->groupKey : QString()), groupData(groupData)
{
}

ModelItemWrapper::ModelItemWrapper(const ModelItemWrapper &other)
    : itemType(other.itemType), groupKey(other.groupKey), fileData(other.fileData), groupData(other.groupData)
{
}

ModelItemWrapper &ModelItemWrapper::operator=(const ModelItemWrapper &other)
{
    if (this != &other) {
        itemType = other.itemType;
        groupKey = other.groupKey;
        fileData = other.fileData;
        groupData = other.groupData;
    }
    return *this;
}

ModelItemWrapper::~ModelItemWrapper()
{
    // Note: We don't own the groupData pointer, so we don't delete it
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
    } else if (itemType == GroupHeaderItem && groupData) {
        // For group headers, provide appropriate data based on role
        switch (role) {
        case Global::kItemIsGroupHeaderType:
            return true;
        case Global::kItemDisplayRole:
        case Global::kItemNameRole:
            return groupData->getHeaderText();
        case Global::kItemGroupHeaderKey:
            return groupData->groupKey;
        default:
            break;
        }
    }

    return QVariant();
}

bool ModelItemWrapper::isValid() const
{
    if (itemType == FileItem) {
        return fileData != nullptr;
    } else if (itemType == GroupHeaderItem) {
        return groupData != nullptr;
    }

    return false;
}

DPWORKSPACE_END_NAMESPACE
