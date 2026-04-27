// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewsorter.h"
#include "models/fileitemdata.h"

#include <dfm-base/utils/filenamesorter.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QStandardPaths>
#include <QVector>
#include <QPair>
#include <QDateTime>

#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

namespace {
// 获取默认时间字符串（用于排序时统一处理无效时间）
inline QString defaultTimeStr()
{
    static const QString kDefaultTime = "0000/00/00 00:00:00";
    return kDefaultTime;
}

// 从 SortInfo 获取时间字符串
QString getDateTimeFromSortInfo(const SortInfoPointer &sortInfo, time_t secs)
{
    if (sortInfo && secs > 0) {
        auto time = QDateTime::fromSecsSinceEpoch(secs);
        if (time.isValid())
            return time.toString(dfmbase::FileUtils::dateTimeFormat());
    }
    return QString();   // 返回空表示需要回退
}

// 从 FileInfo 获取时间字符串
QString getDateTimeFromFileInfo(const FileInfoPointer &fileInfo, dfmbase::TimeInfoType type)
{
    if (fileInfo) {
        auto time = fileInfo->timeOf(type).value<QDateTime>();
        if (time.isValid())
            return time.toString(dfmbase::FileUtils::dateTimeFormat());
    }
    return QString();   // 返回空表示需要回退
}

// MimeType 分组权重映射
const QHash<QString, int> &getMimeTypeGroupMap()
{
    static const QHash<QString, int> map = {
        { "Directory", 0 },
        { "Text", 1 },
        { "Image", 2 },
        { "Video", 3 },
        { "Audio", 4 },
        { "Archive", 5 },
        { "Application", 6 },
        { "Executable", 7 },
        { "Backup file", 8 },
        { "Unknown", 9 }
    };
    return map;
}
}   // namespace

void FileViewSorter::setContext(const SortContext &context)
{
    m_context = context;
}

QList<QUrl> FileViewSorter::sort(const QList<QUrl> &urls)
{
    if (urls.size() <= 1)
        return urls;

    // 非混排状态：目录和文件分开排序，目录始终在前
    if (!m_context.isMixDirAndFile) {
        return sortSeparated(urls);
    }

    // 混排状态：整体排序
    return sortMixed(urls);
}

QList<QUrl> FileViewSorter::sortSeparated(const QList<QUrl> &urls)
{
    // 分离目录和文件
    QList<QUrl> dirs;
    QList<QUrl> files;

    for (const QUrl &url : urls) {
        if (isDir(url)) {
            dirs.append(url);
        } else {
            files.append(url);
        }
    }

    // 分别排序
    QList<QUrl> sortedDirs = sortMixed(dirs);
    QList<QUrl> sortedFiles = sortMixed(files);

    // 合并：目录在前，文件在后
    QList<QUrl> result;
    result.append(sortedDirs);
    result.append(sortedFiles);

    return result;
}

QList<QUrl> FileViewSorter::sortMixed(const QList<QUrl> &urls)
{
    if (urls.size() <= 1)
        return urls;

    // 预生成所有 sortKey（不包含目录/文件前缀）
    QVector<QPair<QUrl, QCollatorSortKey>> items;
    items.reserve(urls.size());

    QCollator &c = collator();
    for (const QUrl &url : urls) {
        // 使用不带目录/文件前缀的 sortKey
        QString keyStr = generateSortKeyStringInternal(url);
        items.append(qMakePair(url, c.sortKey(keyStr)));
    }

    // 使用 FileNameSorter 的排序模板
    dfmbase::FileNameSorter::sortByKey(
            items,
            [](const auto &item) { return item.second; },
            m_context.order);

    // 提取排序后的 URL
    QList<QUrl> result;
    result.reserve(items.size());
    for (const auto &item : items) {
        result.append(item.first);
    }

    return result;
}

QList<QUrl> FileViewSorter::reverse(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return urls;

    // Size 排序时，始终分组处理（无论是否混排）
    // 因为目录和文件的 size 语义不同，不应该混在一起 reverse
    bool needGroupedReverse = !m_context.isMixDirAndFile || (m_context.role == SortRole::Size);

    if (!needGroupedReverse) {
        // 普通混排：直接整体 reverse
        QList<QUrl> result = urls;
        std::reverse(result.begin(), result.end());
        return result;
    }

    // 分组 reverse：目录组和文件组各自内部 reverse
    // 找到第一个文件的位置（目录在前，文件在后）
    int firstFileIndex = -1;
    for (int i = 0; i < urls.size(); ++i) {
        if (!isDir(urls.at(i))) {
            firstFileIndex = i;
            break;
        }
    }

    QList<QUrl> result;

    if (firstFileIndex == -1) {
        // 全部是目录
        result = urls;
        std::reverse(result.begin(), result.end());
    } else if (firstFileIndex == 0) {
        // 全部是文件
        result = urls;
        std::reverse(result.begin(), result.end());
    } else {
        // 有目录也有文件：分别 reverse
        QList<QUrl> dirs = urls.mid(0, firstFileIndex);
        QList<QUrl> files = urls.mid(firstFileIndex);

        std::reverse(dirs.begin(), dirs.end());
        std::reverse(files.begin(), files.end());

        result = dirs;
        result.append(files);
    }

    return result;
}

int FileViewSorter::findInsertPosition(const QUrl &url, const QList<QUrl> &sortedList)
{
    if (sortedList.isEmpty())
        return 0;

    QString newKeyStr = generateSortKeyString(url);
    QCollatorSortKey newKey = collator().sortKey(newKeyStr);

    // 二分查找
    int left = 0;
    int right = sortedList.size();

    while (left < right) {
        int mid = left + (right - left) / 2;
        QString midKeyStr = generateSortKeyString(sortedList.at(mid));
        QCollatorSortKey midKey = collator().sortKey(midKeyStr);

        bool shouldMoveRight = (m_context.order == Qt::AscendingOrder)
                ? (newKey < midKey)
                : (midKey < newKey);

        if (shouldMoveRight) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    return left;
}

FileViewSorter::SortRole FileViewSorter::toItemRole(dfmbase::Global::ItemRoles role)
{
    using namespace dfmbase::Global;
    switch (role) {
    case kItemFileDisplayNameRole:
        return SortRole::FileName;
    case kItemFileSizeRole:
        return SortRole::Size;
    case kItemFileLastModifiedRole:
        return SortRole::LastModified;
    case kItemFileCreatedRole:
        return SortRole::LastCreated;
    case kItemFileLastReadRole:
        return SortRole::LastRead;
    case kItemFileMimeTypeRole:
        return SortRole::MimeType;
    case kItemFilePathRole:
        return SortRole::FilePath;
    case kItemFileOriginalPath:
        return SortRole::OriginalPath;
    case kItemFileDeletionDate:
        return SortRole::DeletionDate;
    default:
        fmWarning() << "FileViewSorter: unsupported sort role:" << role << ", fallback to FileName";
        return SortRole::FileName;
    }
}

QString FileViewSorter::generateSortKeyString(const QUrl &url)
{
    // 获取不带目录/文件前缀的 sortKey
    QString sortKey = generateSortKeyStringInternal(url);

    // 如果是混排状态，直接返回
    if (m_context.isMixDirAndFile) {
        return sortKey;
    }

    // 非混排状态：添加目录/文件前缀
    // 升序时：目录 "0_"，文件 "1_" → 目录在前
    // 降序时：目录 "1_"，文件 "0_" → 目录在后（但由于降序反转，实际上目录仍在视觉上的前面）
    bool isDirectory = isDir(url);
    QString prefix;
    if (m_context.order == Qt::AscendingOrder) {
        prefix = isDirectory ? "0_" : "1_";
    } else {
        prefix = isDirectory ? "1_" : "0_";
    }
    return prefix + sortKey;
}

QString FileViewSorter::generateSortKeyStringInternal(const QUrl &url)
{
    QString sortKey;

    switch (m_context.role) {
    case SortRole::FileName: {
        // 文件名排序
        QString fileName;
        if (m_context.getDataCallback) {
            auto itemData = m_context.getDataCallback(url);
            fileName = getFileDisplayName(url, itemData);
        } else {
            fileName = url.fileName();
        }
        sortKey = fileName;
        break;
    }
    case SortRole::Size: {
        // 大小排序
        qint64 size = 0;
        bool isDirectory = isDir(url);

        if (!isDirectory) {
            QVariant sizeData = getSortData(url);
            size = sizeData.toLongLong();
        }

        // 混排状态：需要根据排序方向决定目录前缀
        // ASC: 目录排前面 → 目录前缀 '0'，文件前缀 '1'
        // DESC: 目录排后面 → 目录前缀 '1'，文件前缀 '0'
        QString prefix;
        if (m_context.isMixDirAndFile) {
            bool dirFirst = (m_context.order == Qt::AscendingOrder);
            if (isDirectory) {
                prefix = dirFirst ? "0_" : "1_";
            } else {
                prefix = dirFirst ? "1_" : "0_";
            }
        } else {
            // 非混排状态不需要前缀（已在 sortSeparated 中分离）
            prefix = "";
        }

        // 使用 19 位数字编码
        QString sizeKey = QString::number(size).rightJustified(19, '0');
        sortKey = prefix + sizeKey + "_" + url.fileName();
        break;
    }
    case SortRole::LastModified:
    case SortRole::LastCreated:
    case SortRole::LastRead:
    case SortRole::DeletionDate: {
        // 时间排序：时间字符串 + 文件名二次排序
        QString timeStr = getSortData(url).toString();
        if (timeStr.isEmpty() || timeStr == "-")
            timeStr = defaultTimeStr();
        sortKey = timeStr + "_" + url.fileName();
        break;
    }
    case SortRole::MimeType: {
        // MimeType 排序：分组编码 + 文件名
        QString mimeType = getSortData(url).toString();
        QString fileName;
        if (m_context.getDataCallback) {
            auto itemData = m_context.getDataCallback(url);
            fileName = getFileDisplayName(url, itemData);
        } else {
            fileName = url.fileName();
        }
        sortKey = encodeMimeTypeSortKey(mimeType, fileName);
        break;
    }
    case SortRole::FilePath:
    case SortRole::OriginalPath: {
        // 路径排序
        QString path = getSortData(url).toString();
        sortKey = path;
        break;
    }
    }

    return sortKey;
}

QVariant FileViewSorter::getSortData(const QUrl &url)
{
    if (!m_context.getDataCallback)
        return QVariant();

    auto itemData = m_context.getDataCallback(url);
    if (!itemData)
        return QVariant();

    FileInfoPointer fileInfo = itemData->fileInfo();
    SortInfoPointer sortInfo = itemData->fileSortInfo();

    using namespace dfmbase::Global;

    switch (m_context.role) {
    case SortRole::FileName:
        return getFileDisplayName(url, itemData);

    case SortRole::Size:
        if (fileInfo)
            return fileInfo->size();
        if (sortInfo)
            return sortInfo->fileSize();
        return 0;

    case SortRole::LastModified: {
        QString time = getDateTimeFromSortInfo(sortInfo, sortInfo ? sortInfo->lastModifiedTime() : 0);
        if (!time.isEmpty())
            return time;
        time = getDateTimeFromFileInfo(fileInfo, dfmbase::TimeInfoType::kLastModified);
        if (!time.isEmpty())
            return time;
        auto createdInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
        time = getDateTimeFromFileInfo(createdInfo, dfmbase::TimeInfoType::kLastModified);
        return time.isEmpty() ? defaultTimeStr() : time;
    }
    case SortRole::LastCreated: {
        QString time = getDateTimeFromSortInfo(sortInfo, sortInfo ? sortInfo->createTime() : 0);
        if (!time.isEmpty())
            return time;
        time = getDateTimeFromFileInfo(fileInfo, dfmbase::TimeInfoType::kCreateTime);
        if (!time.isEmpty())
            return time;
        auto createdInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
        time = getDateTimeFromFileInfo(createdInfo, dfmbase::TimeInfoType::kCreateTime);
        return time.isEmpty() ? defaultTimeStr() : time;
    }
    case SortRole::LastRead: {
        QString time = getDateTimeFromSortInfo(sortInfo, sortInfo ? sortInfo->lastReadTime() : 0);
        if (!time.isEmpty())
            return time;
        time = getDateTimeFromFileInfo(fileInfo, dfmbase::TimeInfoType::kLastRead);
        if (!time.isEmpty())
            return time;
        auto createdInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
        time = getDateTimeFromFileInfo(createdInfo, dfmbase::TimeInfoType::kLastRead);
        return time.isEmpty() ? defaultTimeStr() : time;
    }
    case SortRole::MimeType: {
        // SortInfoPointer 和 FileInfoPointer 使用不同的获取方式
        if (sortInfo) {
            // 内联 getLocalPath + accurateLocalMimeType 逻辑
            QString path;
            if (url.isLocalFile()) {
                path = url.toLocalFile();
            } else {
                auto info = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
                if (info && info->canAttributes(dfmbase::FileInfo::FileCanType::kCanRedirectionFileUrl)) {
                    path = info->urlOf(dfmbase::UrlInfoType::kRedirectedFileUrl).toLocalFile();
                }
            }
            return dfmbase::MimeTypeDisplayManager::instance()->accurateLocalMimeTypeName(path);
        }
        if (fileInfo)
            return fileInfo->displayOf(dfmbase::DisPlayInfoType::kFileTypeDisplayName);
        return "Unknown";
    }

    case SortRole::FilePath:
    case SortRole::OriginalPath:
        // FilePath 和 OriginalPath 只能从 FileInfo 获取
        if (!fileInfo)
            fileInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
        if (fileInfo) {
            if (m_context.role == SortRole::FilePath)
                return fileInfo->displayOf(dfmbase::DisPlayInfoType::kFileDisplayPath);
            else   // OriginalPath
                return fileInfo->urlOf(dfmbase::UrlInfoType::kOriginalUrl).path();
        }
        return url.path();

    case SortRole::DeletionDate:
        // DeletionDate 需要从 FileInfo 的 customData 获取
        if (!fileInfo)
            fileInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
        if (fileInfo)
            return fileInfo->customData(dfmbase::Global::kItemFileDeletionDate);
        return "-";
    }

    return QVariant();
}

int FileViewSorter::getMimeTypeGroupRank(const QString &mimeType)
{
    const auto &map = getMimeTypeGroupMap();

    // 提取主类型（空格前的部分）
    int spacePos = mimeType.indexOf(' ');
    QString majorType = (spacePos == -1) ? mimeType : mimeType.left(spacePos);

    return map.value(majorType, map.value("Unknown"));
}

QString FileViewSorter::encodeMimeTypeSortKey(const QString &mimeType, const QString &fileName)
{
    // 格式：[分组权重1位]_[MimeType]_[文件名]
    // 例如：0_Directory_file, 1_Text_doc, 2_Image_photo
    int rank = getMimeTypeGroupRank(mimeType);
    return QString("%1_%2_%3").arg(rank).arg(mimeType).arg(fileName);
}

bool FileViewSorter::isDir(const QUrl &url)
{
    if (m_context.getDataCallback) {
        auto itemData = m_context.getDataCallback(url);
        if (itemData) {
            auto sortInfo = itemData->fileSortInfo();
            if (sortInfo)
                return sortInfo->isDir();

            auto fileInfo = itemData->fileInfo();
            if (fileInfo)
                return fileInfo->isAttributes(dfmbase::OptInfoType::kIsDir);
        }
    }

    // 回退：尝试从 URL 判断
    return false;
}

QString FileViewSorter::getFileDisplayName(const QUrl &url, const FileItemDataPointer &itemData)
{
    // 根据上下文标志判断是否需要特殊处理
    if (!m_context.isUnderHomeDir && !m_context.checkDesktopFile) {
        return url.fileName();
    }

    // desktop 文件检查（仅当 checkDesktopFile 为 true 时）
    bool isDesktopFile = m_context.checkDesktopFile && dfmbase::FileUtils::isDesktopFileSuffix(url);
    if (!m_context.isUnderHomeDir && !isDesktopFile) {
        return url.fileName();
    }

    // 需要从 FileInfo 获取 displayOf
    FileInfoPointer fileInfo = itemData ? itemData->fileInfo() : nullptr;
    if (!fileInfo) {
        fileInfo = dfmbase::InfoFactory::create<dfmbase::FileInfo>(url);
    }
    if (fileInfo) {
        return fileInfo->displayOf(dfmbase::DisPlayInfoType::kFileDisplayName);
    }

    return url.fileName();
}

QCollator &FileViewSorter::collator()
{
    // 复用 FileNameSorter 的线程安全 collator
    return dfmbase::FileNameSorter::collator();
}

DPWORKSPACE_END_NAMESPACE
