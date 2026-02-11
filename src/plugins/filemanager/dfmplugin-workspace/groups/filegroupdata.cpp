// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filegroupdata.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <algorithm>
#include <optional>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

FileGroupData::FileGroupData()
    : fileCount(0), isExpanded(true), displayOrder(0), displayIndex(0)
{
}

FileGroupData::FileGroupData(const FileGroupData &other)
    : groupKey(other.groupKey),
      displayName(other.displayName),
      fileCount(other.fileCount),
      isExpanded(other.isExpanded),
      displayOrder(other.displayOrder),
      displayIndex(other.displayIndex),
      files(other.files)
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
        displayIndex = other.displayIndex;
        files = other.files;
    }
    return *this;
}

FileGroupData::~FileGroupData()
{
}

QString FileGroupData::getHeaderText() const
{
    return displayName;
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

void FileGroupData::replaceFile(int index, const FileItemDataPointer &file)
{
    if (!file) {
        return;
    }

    // Make sure the index is within valid range
    if (index < 0 || index >= files.size()) {
        return;
    }

    files[index] = file;
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

std::optional<int> FileGroupData::findFileIndex(const QUrl &url) const
{
    if (!url.isValid())
        return std::nullopt;

    for (int i = 0; i < files.size(); ++i) {
        const FileItemDataPointer &file = files.at(i);
        if (file) {
            const auto &fileUrl = file->data(kItemUrlRole).toUrl();
            if (UniversalUtils::urlEquals(fileUrl, url)) {
                return i;
            }
        }
    }

    return std::nullopt;
}
