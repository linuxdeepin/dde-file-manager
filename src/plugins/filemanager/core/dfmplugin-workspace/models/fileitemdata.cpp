// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileitemdata.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <QStandardPaths>

using namespace dfmbase;
using namespace dfmbase::Global;
using namespace dfmplugin_workspace;

FileItemData::FileItemData(const QUrl &url, const FileInfoPointer info, FileItemData *parent)
    : parent(parent),
      url(url),
      info(info)
{
}

FileItemData::FileItemData(const SortInfoPointer &info, FileItemData *parent)
    : parent(parent),
      url(info->url),
      sortInfo(info)
{
}

void FileItemData::setParentData(FileItemData *p)
{
    parent = p;
}

void FileItemData::setSortFileInfo(SortInfoPointer info)
{
    sortInfo = info;
}

void FileItemData::refreshInfo()
{
    if (!info.isNull())
        info->refresh();
}

FileInfoPointer FileItemData::fileInfo() const
{
    if (!info)
        const_cast<FileItemData *>(this)->info = InfoFactory::create<FileInfo>(url);
    return info;
}

FileItemData *FileItemData::parentData() const
{
    return parent;
}

QVariant FileItemData::data(int role) const
{
    if (!info)
        const_cast<FileItemData *>(this)->info = InfoFactory::create<FileInfo>(url);

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
    case kItemSizeHintRole:
        return QSize(-1, 26);
    case kItemNameRole:
        return info->nameOf(NameInfoType::kFileName);
    case Qt::DisplayRole:
    case kItemEditRole:
    case kItemFileDisplayNameRole:
        return info->displayOf(DisPlayInfoType::kFileDisplayName);
    case kItemFileLastReadRole:
        return info->customData(dfmbase::Global::kItemFileLastReadRole);
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
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter;
    case kItemFileIconModelToolTipRole: {
        const QString filePath = data(kItemFilePathRole).toString();
        const QString stdDocPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
        const QString stdDownPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DownloadLocation);
        const QString stdDataDocPath = FileUtils::bindPathTransform(stdDocPath, true);
        const QString stdDataDownPath = FileUtils::bindPathTransform(stdDownPath, true);
        if (filePath == stdDocPath || filePath == stdDownPath || filePath == stdDataDocPath || filePath == stdDataDownPath)
            return QString();

        QString strToolTip = data(kItemFileDisplayNameRole).toString();
        return strToolTip;
    }
    default:
        return QVariant();
    }
}
