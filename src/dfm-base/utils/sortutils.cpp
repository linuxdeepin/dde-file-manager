// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

bool isNumOrChar(const QChar ch)
{
    QChar normalized;
    if (isFullWidthChar(ch, normalized))
        return isNumOrChar(normalized);

    auto chValue = ch.unicode();
    return (chValue >= 48 && chValue <= 57) || (chValue >= 65 && chValue <= 90) || (chValue >= 97 && chValue <= 122);
}

bool isNumber(const QChar ch)
{
    QChar number;
    // 全角数字也被认为是数字
    if (isFullWidthChar(ch, number)) {
        return (number.unicode() >= '0' && number.unicode() <= '9');
    }
    return ch.isDigit();
}

bool isSymbol(const QChar ch)
{
    // 如果是高代理项，不应该单独判断
    if (ch.isHighSurrogate() || ch.isLowSurrogate())
        return false;

    QChar normalized;
    if (isFullWidthChar(ch, normalized)) {
        return isSymbol(normalized);
    }

    // 对于普通字符进行原有判断
    return ch.script() != QChar::Script_Han && !isNumOrChar(ch);
}

QString numberStr(const QString &str, int pos)
{
    QString tmp;
    int total = str.length();

    // 从当前'pos'开始向前扫描
    while (pos < total) {
        const QChar &ch = str.at(pos);
        if (!isNumber(ch)) {
            break;   // 遇到非数字字符，停止提取
        }

        QChar number;
        // 将全角数字统一转换为半角数字后添加
        if (isFullWidthChar(ch, number)) {
            tmp += number;
        } else {
            tmp += ch;
        }
        pos++;
    }

    return tmp;
}

bool isFullWidthChar(const QChar ch, QChar &normalized)
{
    // 全角字符的 Unicode 范围
    ushort unicode = ch.unicode();

    // 处理全角数字 (0xFF10-0xFF19)
    if (unicode >= 0xFF10 && unicode <= 0xFF19) {
        normalized = QChar(unicode - 0xFF10 + '0');
        return true;
    }

    // 处理全角大写字母 (0xFF21-0xFF3A)
    if (unicode >= 0xFF21 && unicode <= 0xFF3A) {
        normalized = QChar(unicode - 0xFF21 + 'A');
        return true;
    }

    // 处理全角小写字母 (0xFF41-0xFF5A)
    if (unicode >= 0xFF41 && unicode <= 0xFF5A) {
        normalized = QChar(unicode - 0xFF41 + 'a');
        return true;
    }

    // 处理全角标点符号
    static const QHash<ushort, QChar> punctuationMap {
        { 0xFF01, '!' },   // ！
        { 0xFF08, '(' },   // （
        { 0xFF09, ')' },   // ）
        { 0xFF0C, ',' },   // ，
        { 0xFF1A, ':' },   // ：
        { 0xFF1B, ';' },   // ；
        { 0xFF1F, '?' },   // ？
        { 0xFF3B, '[' },   // ［
        { 0xFF3D, ']' },   // ］
        { 0xFF5B, '{' },   // ｛
        { 0xFF5D, '}' },   // ｝
        { 0xFF0E, '.' },   // ．
        { 0xFF0F, '/' },   // ／
        { 0xFF3F, '_' },   // ＿
        { 0xFF0D, '-' },   // －
        { 0xFF1D, '=' },   // ＝
        { 0xFF06, '&' },   // ＆
        { 0xFF5C, '|' },   // ｜
        { 0xFF1C, '<' },   // ＜
        { 0xFF1E, '>' },   // ＞
        { 0xFF02, '"' },   // ＂
        { 0xFF07, '\'' },   // ＇
        { 0xFF0B, '+' },   // ＋
        { 0xFF03, '#' },   // ＃
        { 0xFF04, '$' },   // ＄
        { 0xFF05, '%' },   // ％
        { 0xFF20, '@' },   // ＠
        { 0xFF0A, '*' },   // ＊
        { 0xFF3C, '\\' },   // ＼
        { 0xFF5E, '~' }   // ～
    };

    auto it = punctuationMap.find(unicode);
    if (it != punctuationMap.end()) {
        normalized = it.value();
        return true;
    }

    return false;
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

QString fastDisplayType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->fastDisplayTypeFromPath(path);
}

QString fastMimeType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->fastMimeTypeName(path);
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
