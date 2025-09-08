 // SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sizegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QDebug>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

// Size order according to requirements (from smallest to largest)
const QStringList SizeGroupStrategy::SIZE_ORDER = {
    "unknown",      // 未知（文件夹）
    "empty",        // 空（0kb）
    "tiny",         // 极小（0-16kb）
    "small",        // 小（16kb-1MB）
    "medium",       // 中等（1-128MB）  
    "large",        // 大（128MB-1GB）
    "huge",         // 巨大（1-4GB）
    "gigantic"      // 超大（>4GB）
};

// Display names for each size group with size ranges
const QHash<QString, QString> SizeGroupStrategy::DISPLAY_NAMES = {
    {"unknown", QObject::tr("Unknown")},
    {"empty", QObject::tr("Empty (0KB)")},
    {"tiny", QObject::tr("Tiny (0-16KB)")},
    {"small", QObject::tr("Small (16KB-1MB)")},
    {"medium", QObject::tr("Medium (1-128MB)")},
    {"large", QObject::tr("Large (128MB-1GB)")},
    {"huge", QObject::tr("Huge (1-4GB)")},
    {"gigantic", QObject::tr("Gigantic (>4GB)")}
};

SizeGroupStrategy::SizeGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "SizeGroupStrategy: Initialized";
}

SizeGroupStrategy::~SizeGroupStrategy()
{
    fmDebug() << "SizeGroupStrategy: Destroyed";
}

QString SizeGroupStrategy::getGroupKey(const FileItemDataPointer &item) const
{
    if (!item || !item->fileInfo()) {
        fmWarning() << "SizeGroupStrategy: Invalid item or fileInfo";
        return "unknown";
    }

    const auto fileInfo = item->fileInfo();
    
    // Check if it's a directory - directories have unknown size
    if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
        return "unknown";
    }

    // Get file size and classify
    qint64 size = fileInfo->size();
    QString groupKey = classifyBySize(size);
    
    fmDebug() << "SizeGroupStrategy: File" << fileInfo->urlOf(UrlInfoType::kUrl).toString() 
              << "size:" << size << "bytes -> group:" << groupKey;
    
    return groupKey;
}

QString SizeGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    return DISPLAY_NAMES.value(groupKey, groupKey);
}

QStringList SizeGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    if (order == Qt::AscendingOrder) {
        return SIZE_ORDER;
    } else {
        QStringList reversed = SIZE_ORDER;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
}

int SizeGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    int index = SIZE_ORDER.indexOf(groupKey);
    if (index == -1) {
        index = SIZE_ORDER.size(); // Unknown groups go to the end
    }
    
    if (order == Qt::AscendingOrder) {
        return index;
    } else {
        return SIZE_ORDER.size() - index - 1;
    }
}

bool SizeGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileItemDataPointer> &items) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one item
    return !items.isEmpty();
}

QString SizeGroupStrategy::getStrategyName() const
{
    return tr("Size");
}

Global::ItemRoles SizeGroupStrategy::getCorrespondingRole() const
{
    return Global::ItemRoles::kItemFileSizeRole;
}

QString SizeGroupStrategy::classifyBySize(qint64 size) const
{
    // Define size constants
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    // Classify by size ranges according to requirements
    if (size == 0) {
        return "empty";
    } else if (size <= 16 * KB) {
        return "tiny";
    } else if (size <= 1 * MB) {
        return "small";
    } else if (size <= 128 * MB) {
        return "medium";
    } else if (size <= 1 * GB) {
        return "large";  
    } else if (size <= 4 * GB) {
        return "huge";
    } else {
        return "gigantic";
    }
}

