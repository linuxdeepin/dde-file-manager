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

QStringList TypeGroupStrategy::getTypeOrder()
{
    return {
        "directory",   // 目录
        "document",   // 文档
        "image",   // 图片
        "video",   // 影片
        "audio",   // 音乐
        "archive",   // 压缩文件
        "application",   // 应用程序
        "executable",   // 可执行程序
        "unknown"   // 未知
    };
}

QHash<QString, QString> TypeGroupStrategy::getDisplayNames()
{
    return {
        { "directory", QObject::tr("Directory") },
        { "document", QObject::tr("Text") },
        { "image", QObject::tr("Image") },
        { "video", QObject::tr("Video") },
        { "audio", QObject::tr("Audio") },
        { "archive", QObject::tr("Archive") },
        { "application", QObject::tr("Application") },
        { "executable", QObject::tr("Executable") },
        { "unknown", QObject::tr("Unknown") }
    };
}

TypeGroupStrategy::TypeGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "TypeGroupStrategy: Initialized";
}

TypeGroupStrategy::~TypeGroupStrategy()
{
    fmDebug() << "TypeGroupStrategy: Destroyed";
}

QString TypeGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    if (!info) {
        fmWarning() << "TypeGroupStrategy: Invalid fileInfo";
        return "unknown";
    }

    // Check if it's a directory first
    if (info->isAttributes(OptInfoType::kIsDir)) {
        return "directory";
    }

    // Get MIME type and map to group
    QString mimeType = info->nameOf(NameInfoType::kMimeTypeName);
    if (mimeType.isEmpty()) {
        // Fallback: try to get MIME type from file extension
        mimeType = info->fileMimeType().name();
    }

    QString groupKey = mapMimeTypeToGroup(mimeType);

    fmDebug() << "TypeGroupStrategy: File" << info->urlOf(UrlInfoType::kUrl).toString()
              << "MIME type:" << mimeType << "-> group:" << groupKey;

    return groupKey;
}

QString TypeGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    return getDisplayNames().value(groupKey, groupKey);
}

QStringList TypeGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    QStringList typeOrder = getTypeOrder();
    if (order == Qt::AscendingOrder) {
        return typeOrder;
    } else {
        QStringList reversed = typeOrder;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
}

int TypeGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    QStringList typeOrder = getTypeOrder();
    int index = typeOrder.indexOf(groupKey);
    if (index == -1) {
        index = typeOrder.size();   // Unknown types go to the end
    }

    if (order == Qt::AscendingOrder) {
        return index;
    } else {
        return typeOrder.size() - index - 1;
    }
}

bool TypeGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one file info
    return !infos.isEmpty();
}

QString TypeGroupStrategy::getStrategyName() const
{
    return GroupStrategty::kType;
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
        return "archive";   // Backup files are classified as archive files
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
