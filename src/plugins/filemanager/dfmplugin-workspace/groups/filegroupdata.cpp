// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filegroupdata.h"

#include <QUrl>
#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE

FileGroupData::FileGroupData()
    : fileCount(0)
    , isExpanded(true)
    , displayOrder(0)
{
}

FileGroupData::FileGroupData(const FileGroupData &other)
    : groupKey(other.groupKey)
    , displayName(other.displayName)
    , fileCount(other.fileCount)
    , isExpanded(other.isExpanded)
    , displayOrder(other.displayOrder)
    , files(other.files)
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

    // Check if file already exists to avoid duplicates
    const QUrl fileUrl = file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
    for (const auto &existingFile : files) {
        if (existingFile && existingFile->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl() == fileUrl) {
            return; // File already exists in this group
        }
    }

    files.append(file);
    updateFileCount();
}

bool FileGroupData::removeFile(const QUrl &url)
{
    auto it = std::find_if(files.begin(), files.end(),
                          [&url](const FileItemDataPointer &file) {
                              return file && file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl() == url;
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

void FileGroupData::sortFiles(const std::function<bool(const FileItemDataPointer&, const FileItemDataPointer&)> &lessThan)
{
    if (!lessThan) {
        return;
    }

    std::sort(files.begin(), files.end(), lessThan);
}

void FileGroupData::updateFileCount()
{
    // Filter out null pointers when counting
    fileCount = 0;
    for (const auto &file : files) {
        if (file) {
            ++fileCount;
        }
    }
}

DPWORKSPACE_END_NAMESPACE 