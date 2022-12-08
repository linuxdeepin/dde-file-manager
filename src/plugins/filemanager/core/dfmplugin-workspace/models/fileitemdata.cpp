/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "fileitemdata.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <QStandardPaths>

using namespace dfmbase;
using namespace dfmbase::Global;
using namespace dfmplugin_workspace;

FileItemData::FileItemData(const QUrl &url, FileItemData *parent)
    : QObject(parent),
      parent(parent),
      info(InfoFactory::create<AbstractFileInfo>(url))
{
}

void FileItemData::setParentData(FileItemData *p)
{
    parent = p;
}

void FileItemData::refreshInfo()
{
    if (!info.isNull())
        info->refresh();
}

AbstractFileInfoPointer FileItemData::fileInfo() const
{
    return info;
}

FileItemData *FileItemData::parentData() const
{
    return parent;
}

QVariant FileItemData::data(int role) const
{
    if (info.isNull())
        return QVariant();

    auto val = info->customData(role);
    if (val.isValid())
        return val;

    switch (role) {
    case kItemFilePathRole:
        return info->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    case kItemFileLastModifiedRole: {
        auto lastModified = info->timeInfo(AbstractFileInfo::FileTimeType::kLastModified).value<QDateTime>();
        return lastModified.isValid() ? lastModified.toString(FileUtils::dateTimeFormat()) : "-";
    }
    case kItemIconRole:
        return info->fileIcon();
    case kItemFileSizeRole:
        return info->displayInfo(AbstractFileInfo::DisplayInfoType::kSizeDisplayName);
    case kItemFileMimeTypeRole:
        return info->displayInfo(AbstractFileInfo::DisplayInfoType::kMimeTypeDisplayName);
    case kItemSizeHintRole:
        return QSize(-1, 26);
    case kItemNameRole:
        return info->nameInfo(NameInfo::kFileName);
    case Qt::DisplayRole:
    case kItemEditRole:
    case kItemFileDisplayNameRole:
        return info->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    case kItemFilePinyinNameRole:
        return info->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayPinyinName);
    case kItemFileBaseNameRole:
        return info->nameInfo(NameInfo::kCompleteBaseName);
    case kItemFileSuffixRole:
        return info->nameInfo(NameInfo::kSuffix);
    case kItemFileNameOfRenameRole:
        return info->nameInfo(NameInfo::kFileNameOfRename);
    case kItemFileBaseNameOfRenameRole:
        return info->nameInfo(NameInfo::kBaseNameOfRename);
    case kItemFileSuffixOfRenameRole:
        return info->nameInfo(NameInfo::kSuffixOfRename);
    case kItemUrlRole:
        return info->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl);
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
