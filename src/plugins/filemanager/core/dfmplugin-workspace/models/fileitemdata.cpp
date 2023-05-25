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

FileItemData::FileItemData(const QUrl &url, const FileInfoPointer &info, FileItemData *parent)
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
    if (info) {
        auto val = info->customData(role);
        if (val.isValid())
            return val;
    }

    switch (role) {
    case kItemCreateFileInfo:
        if (info.isNull())
            const_cast<FileItemData *>(this)->info = InfoFactory::create<FileInfo>(url);
        return QVariant();
    case kItemFilePathRole:
        if (info)
            return info->displayOf(DisPlayInfoType::kFileDisplayPath);
        return url.path();
    case kItemFileLastModifiedRole: {
        if (info) {
            auto lastModified = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
            return lastModified.isValid() ? lastModified.toString(FileUtils::dateTimeFormat()) : "-";
        }
        return "-";
    }
    case kItemIconRole:
        if (info)
            return info->fileIcon();
        return QIcon::fromTheme("unknown");
    case kItemFileSizeRole:
        if (info)
            return info->displayOf(DisPlayInfoType::kSizeDisplayName);
        return "-";
    case kItemFileMimeTypeRole:
        if (info)
            return info->displayOf(DisPlayInfoType::kMimeTypeDisplayName);
        return QString();
    case kItemSizeHintRole:
        return QSize(-1, 26);
    case kItemNameRole:
        if (info)
            return info->nameOf(NameInfoType::kFileName);
        return url.fileName();
    case Qt::DisplayRole:
    case kItemEditRole:
    case kItemFileDisplayNameRole:
        if (info)
            return info->displayOf(DisPlayInfoType::kFileDisplayName);
        return QString();
    case kItemFileLastReadRole:
        if (info)
            return info->customData(dfmbase::Global::kItemFileLastReadRole);
        return QString();
    case kItemFilePinyinNameRole:
        if (info)
            return info->displayOf(DisPlayInfoType::kFileDisplayPinyinName);
        return url.fileName();
    case kItemFileBaseNameRole:
        if (info)
            return info->nameOf(NameInfoType::kCompleteBaseName);
        return url.fileName();
    case kItemFileSuffixRole:
        if (info)
            return info->nameOf(NameInfoType::kSuffix);
        return url.fileName();
    case kItemFileNameOfRenameRole:
        if (info)
            return info->nameOf(NameInfoType::kFileNameOfRename);
        return url.fileName();
    case kItemFileBaseNameOfRenameRole:
        if (info)
            return info->nameOf(NameInfoType::kBaseNameOfRename);
        return url.fileName();
    case kItemFileSuffixOfRenameRole:
        if (info)
            return info->nameOf(NameInfoType::kSuffixOfRename);
        return url.fileName();
    case kItemUrlRole:
        if (info)
            return info->urlOf(UrlInfoType::kUrl);
        return url;
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
    case kItemFileIsWritable:
        if (info)
            return info->isAttributes(OptInfoType::kIsWritable);
        if (sortInfo)
            return sortInfo->isWriteable;
        return true;
    case kItemFileIsDir:
        if (info)
            return info->isAttributes(OptInfoType::kIsDir);
        if (sortInfo)
            return sortInfo->isDir;
        return true;
    case kItemFileCanRename:
        if (info)
            return info->canAttributes(CanableInfoType::kCanRename);
        return true;
    case kItemFileCanDrop:
        if (info)
            return info->canAttributes(CanableInfoType::kCanDrop);
        return true;
    case kItemFileCanDrag:
        if (info)
            return info->canAttributes(CanableInfoType::kCanDrag);
        return true;
    default:
        return QVariant();
    }
}
