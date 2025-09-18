// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filegroupdata.h"

#include <dfm-base/utils/universalutils.h>

#include <QUrl>

#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

FileGroupData::FileGroupData()
    : fileCount(0), isExpanded(true), displayOrder(0)
{
}

FileGroupData::FileGroupData(const FileGroupData &other)
    : groupKey(other.groupKey), displayName(other.displayName), fileCount(other.fileCount), isExpanded(other.isExpanded), displayOrder(other.displayOrder), files(other.files)
{
}

FileGroupData &FileGroupData::operator=(const FileGroupData &other)
{
    if (this != &other) {
        groupKey = other.groupKey;
        displayName = other.displayName;
        fileCount = other.fileCount;
        isExpanded = other.isExpanded;
        displayOrder = other.displayOrder;
        files = other.files;
    }
    return *this;
}

FileGroupData::~FileGroupData()
{
}

QString FileGroupData::getHeaderText() const
{
    if (displayName.isEmpty()) {
        return QString("(%1)").arg(fileCount);
    }
    return QString("%1 (%2)").arg(displayName).arg(fileCount);
}

void FileGroupData::addFile(const FileItemDataPointer &file)
{
    if (!file) {
        return;
    }

    files.append(file);
    updateFileCount();
}

void FileGroupData::insertFile(int index, const FileItemDataPointer &file)
{
    if (!file) {
        return;
    }

    // Make sure the index is within valid range
    if (index < 0 || index > files.size()) {
        return;
    }

    files.insert(index, file);
    updateFileCount();
}

bool FileGroupData::removeFile(const QUrl &url)
{
    auto it = std::find_if(files.begin(), files.end(),
                           [&url](const FileItemDataPointer &file) {
                               return file
                                       && UniversalUtils::urlEquals(
                                               file->data(Global::kItemUrlRole).toUrl(),
                                               url);
                           });

    if (it != files.end()) {
        files.erase(it);
        updateFileCount();
        return true;
    }

    return false;
}

void FileGroupData::clear()
{
    files.clear();
    fileCount = 0;
}

bool FileGroupData::isEmpty() const
{
    return files.isEmpty();
}

void FileGroupData::sortFiles(const std::function<bool(const FileItemDataPointer &, const FileItemDataPointer &)> &lessThan)
{
    if (!lessThan) {
        return;
    }

    std::sort(files.begin(), files.end(), lessThan);
}

void FileGroupData::updateFileCount()
{
    fileCount = files.count();
}

DPWORKSPACE_END_NAMESPACE