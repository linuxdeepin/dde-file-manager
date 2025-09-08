 // SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "typegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QDebug>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

// Type order according to requirements document (directories first, then by frequency)
const QStringList TypeGroupStrategy::TYPE_ORDER = {
    "directory",      // 目录
    "document",       // 文档  
    "image",          // 图片
    "video",          // 影片
    "audio",          // 音乐
    "archive",        // 压缩文件
    "application",    // 应用程序
    "executable",     // 可执行程序
    "unknown"         // 未知
};

// Display names for each type group (internationalized)
const QHash<QString, QString> TypeGroupStrategy::DISPLAY_NAMES = {
    {"directory", QObject::tr("Directory")},
    {"document", QObject::tr("Document")}, 
    {"image", QObject::tr("Image")},
    {"video", QObject::tr("Video")},
    {"audio", QObject::tr("Audio")},
    {"archive", QObject::tr("Archive")},
    {"application", QObject::tr("Application")},
    {"executable", QObject::tr("Executable")},
    {"unknown", QObject::tr("Unknown")}
};

TypeGroupStrategy::TypeGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "TypeGroupStrategy: Initialized";
}

TypeGroupStrategy::~TypeGroupStrategy()
{
    fmDebug() << "TypeGroupStrategy: Destroyed";
}

QString TypeGroupStrategy::getGroupKey(const FileItemDataPointer &item) const
{
    if (!item || !item->fileInfo()) {
        fmWarning() << "TypeGroupStrategy: Invalid item or fileInfo";
        return "unknown";
    }

    const auto fileInfo = item->fileInfo();
    
    // Check if it's a directory first
    if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
        return "directory";
    }

    // Get MIME type and map to group
    QString mimeType = fileInfo->nameOf(NameInfoType::kMimeTypeName);
    if (mimeType.isEmpty()) {
        // Fallback: try to get MIME type from file extension
        mimeType = fileInfo->fileMimeType().name();
    }

    QString groupKey = mapMimeTypeToGroup(mimeType);
    
    fmDebug() << "TypeGroupStrategy: File" << fileInfo->urlOf(UrlInfoType::kUrl).toString() 
              << "MIME type:" << mimeType << "-> group:" << groupKey;
    
    return groupKey;
}

QString TypeGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    return DISPLAY_NAMES.value(groupKey, groupKey);
}

QStringList TypeGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    if (order == Qt::AscendingOrder) {
        return TYPE_ORDER;
    } else {
        QStringList reversed = TYPE_ORDER;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
}

int TypeGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    int index = TYPE_ORDER.indexOf(groupKey);
    if (index == -1) {
        index = TYPE_ORDER.size(); // Unknown types go to the end
    }
    
    if (order == Qt::AscendingOrder) {
        return index;
    } else {
        return TYPE_ORDER.size() - index - 1;
    }
}

bool TypeGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileItemDataPointer> &items) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one item
    return !items.isEmpty();
}

QString TypeGroupStrategy::getStrategyName() const
{
    return tr("Type");
}

Global::ItemRoles TypeGroupStrategy::getCorrespondingRole() const
{
    return Global::ItemRoles::kItemFileMimeTypeRole;
}

QString TypeGroupStrategy::mapMimeTypeToGroup(const QString &mimeType) const
{
    if (mimeType.isEmpty()) {
        return "unknown";
    }

    // Use MimeTypeDisplayManager to get the file type classification
    auto displayType = MimeTypeDisplayManager::instance()->displayNameToEnum(mimeType);
    
    switch (displayType) {
    case FileInfo::FileType::kDirectory:
        return "directory";
    case FileInfo::FileType::kDocuments:
        return "document";
    case FileInfo::FileType::kImages:
        return "image";
    case FileInfo::FileType::kVideos:
        return "video";  
    case FileInfo::FileType::kAudios:
        return "audio";
    case FileInfo::FileType::kArchives:
        return "archive";
    case FileInfo::FileType::kExecutable:
        return "executable";
    case FileInfo::FileType::kBackups:
        return "archive";  // Backup files are classified as archive files
    case FileInfo::FileType::kDesktopApplication:
        return "application";
    default:
        // For other application types or unknown
        if (mimeType.startsWith("application/")) {
            return "application";
        }
        return "unknown";
    }
}

