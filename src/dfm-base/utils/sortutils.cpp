// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <dfm-io/dfmio_utils.h>

#include <QCollator>
#include <QChar>

DFMBASE_BEGIN_NAMESPACE

namespace SortUtils {

// fix 多线程排序时，该处的全局变量在compareByString函数中可能导致软件崩溃
// QCollator sortCollator;
class DCollator : public QCollator
{
public:
    DCollator()
        : QCollator()
    {
        setNumericMode(true);
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    return !((order == Qt::AscendingOrder) ^ compareStringForFileName(str1, str2));
}

bool compareStringForFileName(const QString &str1, const QString &str2)
{
    return DFMIO::DFMUtils::compareFileName(str1, str2);
}

bool compareStringForTime(const QString &str1, const QString &str2)
{
    // 固定的、期望的字符串长度
    constexpr int expectedLength = 19;

    // --- Fast Path ---
    // 快速检查两个字符串是否都符合标准长度。
    // 这是最高效的验证，可以过滤掉绝大多数异常情况。
    if (str1.length() == expectedLength && str2.length() == expectedLength) {
        // 由于格式 "YYYY/MM/DD hh:mm:ss" 是字典序可排序的，
        // 直接使用 QString::operator< 进行比较。
        // 这是性能最高的方式，通常会优化为底层的 memcmp。
        return str1 < str2;
    }

    // --- Slow Path (Fallback) ---
    // 如果任何一个字符串长度不符合，我们视其为异常情况，
    // 回退到提供的默认比较函数。
    return compareStringForFileName(str1, str2);
}

qint64 getEffectiveSize(const SortInfoPointer &info)
{
    if (info->isDir()) {
        return -1;
    }

    qint64 size = info->fileSize();
    if (info->isSymLink()) {
        const FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(info->fileUrl());
        if (fileInfo) {
            size = fileInfo->size();
        }
    }
    return size;
}

bool compareStringForMimeType(const QString &str1, const QString &str2)
{
    // 使用立即执行的lambda表达式初始化静态哈希表，确保只执行一次。
    static const QHash<QString, int> typeRankMap = [] {
        QHash<QString, int> map;
        int rank = 0;
        map.insert("Directory", rank++);   // 0
        map.insert("Text", rank++);   // 1
        map.insert("Image", rank++);   // 2
        map.insert("Video", rank++);   // 3
        map.insert("Audio", rank++);   // 4
        map.insert("Archive", rank++);   // 5
        map.insert("Application", rank++);   // 6
        map.insert("Executable", rank++);   // 7
        map.insert("Backup file", rank++);   // 8
        map.insert("Unknown", rank++);   // 9
        return map;
    }();

    // 缓存 "Unknown" 类型的排名，用于处理未识别的类型
    static const int unknownRank = typeRankMap.value("Unknown");

    // --- 为 str1 提取主类型并获取排名 ---
    int spacePos1 = str1.indexOf(' ');
    // 如果没有空格，整个字符串是主类型；否则，取空格前部分
    const QString majorType1 = (spacePos1 == -1) ? str1 : str1.left(spacePos1);
    const int rank1 = typeRankMap.value(majorType1, unknownRank);

    // --- 为 str2 提取主类型并获取排名 ---
    int spacePos2 = str2.indexOf(' ');
    const QString majorType2 = (spacePos2 == -1) ? str2 : str2.left(spacePos2);
    const int rank2 = typeRankMap.value(majorType2, unknownRank);

    // --- 比较 ---
    if (rank1 != rank2) {
        // 主类型不同，按排名排序
        return rank1 < rank2;
    } else {
        // 主类型相同，按次要规则（默认字符串比较）排序
        return compareStringForFileName(str1, str2);
    }
}

bool compareForSize(const SortInfoPointer info1, const SortInfoPointer info2)
{
    qint64 size1 = getEffectiveSize(info1);
    qint64 size2 = getEffectiveSize(info2);

    if (size1 == size2) {
        QString leftName = info1->fileUrl().fileName();
        QString rightName = info2->fileUrl().fileName();
        return SortUtils::compareStringForFileName(leftName, rightName);
    }

    return size1 < size2;
}

bool compareForSize(const qint64 size1, const qint64 size2)
{
    return size1 < size2;
}

QString accurateDisplayType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->accurateDisplayTypeFromPath(path);
}

QString accurateLocalMimeType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->accurateLocalMimeTypeName(path);
}

QString getLocalPath(const QUrl &url)
{
    QString path;
    if (url.isLocalFile()) {
        path = url.toLocalFile();
    } else {
        auto info { InfoFactory::create<FileInfo>(url) };
        if (info && info->canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl)) {
            path = info->urlOf(UrlInfoType::kRedirectedFileUrl).toLocalFile();
        }
    }

    return path;
}

}   // namespace SortUtils

DFMBASE_END_NAMESPACE
