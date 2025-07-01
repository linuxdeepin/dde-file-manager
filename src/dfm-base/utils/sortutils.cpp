// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QCollator>

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

bool compareStringDefault(const QString &str1, const QString &str2)
{
    thread_local static DCollator sortCollator;
    QString suf1 = str1.right(str1.length() - str1.lastIndexOf(".") - 1);
    QString suf2 = str2.right(str2.length() - str2.lastIndexOf(".") - 1);
    QString name1 = str1.left(str1.lastIndexOf("."));
    QString name2 = str2.left(str2.lastIndexOf("."));
    int length1 = name1.length();
    int length2 = name2.length();
    auto total = length1 > length2 ? length2 : length1;

    bool preIsNum = false;
    bool isSybol1 = false, isSybol2 = false, isHanzi1 = false,
         isHanzi2 = false, isNumb1 = false, isNumb2 = false;
    for (int i = 0; i < total; ++i) {
        // 判断相等和大小写相等，跳过
        if (str1.at(i) == str2.at(i) || str1.at(i).toLower() == str2.at(i).toLower()) {
            preIsNum = isNumber(str1.at(i));
            continue;
        }
        isNumb1 = isNumber(str1.at(i));
        isNumb2 = isNumber(str2.at(i));
        if ((preIsNum && (isNumb1 ^ isNumb2)) || (isNumb1 && isNumb2)) {
            // 取后面几位的数字作比较后面的数字,先比较位数
            // 位数大的大
            auto str1n = numberStr(str1, preIsNum ? i - 1 : i).toUInt();
            auto str2n = numberStr(str2, preIsNum ? i - 1 : i).toUInt();
            if (str1n == str2n)
                return str1.at(i) < str2.at(i);
            return str1n < str2n;
        }

        // 判断特殊字符就排到最后
        isSybol1 = isSymbol(str1.at(i));
        isSybol2 = isSymbol(str2.at(i));
        if (isSybol1 ^ isSybol2)
            return !isSybol1;

        if (isSybol1)
            return str1.at(i) < str2.at(i);

        // 判断汉字
        isHanzi1 = str1.at(i).script() == QChar::Script_Han;
        isHanzi2 = str2.at(i).script() == QChar::Script_Han;
        if (isHanzi2 ^ isHanzi1)
            return !isHanzi1;

        if (isHanzi1)
            return sortCollator.compare(str1.at(i), str2.at(i)) < 0;

        // 判断数字或者字符
        if (!isNumb1 && !isNumb2)
            return str1.at(i).toLower() < str2.at(i).toLower();

        return isNumb1;
    }

    if (length1 == length2) {
        if (suf1.isEmpty() ^ suf2.isEmpty())
            return suf1.isEmpty();

        if (suf2.startsWith(suf1) ^ suf1.startsWith(suf2))
            return suf2.startsWith(suf1);

        return suf1 < suf2;
    }

    return length1 < length2;
}

bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    return !((order == Qt::AscendingOrder) ^ compareStringDefault(str1, str2));
}

bool compareStringForFileName(const QString &str1, const QString &str2)
{
    thread_local static DCollator sortCollator;

    // ======================== FIX START ========================
    // 修复文件名和后缀的拆分逻辑
    QString name1, suf1;
    // 查找最后一个'.'的位置
    int dotPos1 = str1.lastIndexOf('.');
    // 如果'.'不存在(返回-1)或在开头(返回0)，则认为没有后缀
    if (dotPos1 <= 0) {
        name1 = str1;
        suf1 = QString();   // 空后缀
    } else {
        name1 = str1.left(dotPos1);
        suf1 = str1.mid(dotPos1 + 1);   // 使用mid更清晰安全
    }

    QString name2, suf2;
    int dotPos2 = str2.lastIndexOf('.');
    if (dotPos2 <= 0) {
        name2 = str2;
        suf2 = QString();
    } else {
        name2 = str2.left(dotPos2);
        suf2 = str2.mid(dotPos2 + 1);
    }
    // ========================= FIX END =========================

    // 后续的自然排序逻辑保持不变，现在它将基于正确拆分的 name1/name2 进行操作
    bool preIsNum = false;
    bool isSymbol1 = false, isSymbol2 = false, isHanzi1 = false,
         isHanzi2 = false, isNumb1 = false, isNumb2 = false;

    // 使用迭代器来正确处理代理对字符
    QString::const_iterator it1 = name1.constBegin();
    QString::const_iterator it2 = name2.constBegin();

    while (it1 != name1.constEnd() && it2 != name2.constEnd()) {
        // ... (原函数的剩余部分完全不变) ...
        // 获取当前完整字符(可能是代理对)
        uint unicode1 = it1->isHighSurrogate() && (it1 + 1) != name1.constEnd()
                ? QChar::surrogateToUcs4(*it1, *(it1 + 1))
                : it1->unicode();
        uint unicode2 = it2->isHighSurrogate() && (it2 + 1) != name2.constEnd()
                ? QChar::surrogateToUcs4(*it2, *(it2 + 1))
                : it2->unicode();

        // 如果字符相同，继续比较下一个
        if (unicode1 == unicode2) {
            preIsNum = isNumber(*it1);
            if (it1->isHighSurrogate()) {
                ++it1;
                ++it2;
            }
            ++it1;
            ++it2;
            continue;
        }

        // 处理数字
        QChar number1, number2;
        isNumb1 = !it1->isHighSurrogate() && isNumber(*it1);
        isNumb2 = !it2->isHighSurrogate() && isNumber(*it2);

        if ((preIsNum && (isNumb1 ^ isNumb2)) || (isNumb1 && isNumb2)) {
            auto str1n = numberStr(name1, it1 - name1.constBegin()).toUInt();
            auto str2n = numberStr(name2, it2 - name2.constBegin()).toUInt();
            if (str1n == str2n) {
                // 如果数值相同，全角数字排在半角数字后面
                bool isFullWidth1 = isFullWidthChar(*it1, number1);
                bool isFullWidth2 = isFullWidthChar(*it2, number2);
                if (isFullWidth1 != isFullWidth2)
                    return !isFullWidth1;
                return unicode1 < unicode2;
            }
            return str1n < str2n;
        }

        // 处理特殊字符
        QChar normalized1, normalized2;
        bool isFullWidth1 = isFullWidthChar(*it1, normalized1);
        bool isFullWidth2 = isFullWidthChar(*it2, normalized2);

        isSymbol1 = !it1->isHighSurrogate() && isSymbol(*it1);
        isSymbol2 = !it2->isHighSurrogate() && isSymbol(*it2);

        // 如果都是符号，先比较归一化后的字符
        if (isSymbol1 && isSymbol2) {
            QChar ch1 = isFullWidth1 ? normalized1 : *it1;
            QChar ch2 = isFullWidth2 ? normalized2 : *it2;
            if (ch1 != ch2)
                return ch1 < ch2;
            // 如果归一化后相同，全角排在半角后面
            if (isFullWidth1 != isFullWidth2)
                return !isFullWidth1;
            // 如果全半角属性也相同，继续比较下一个字符
            if (it1->isHighSurrogate()) {
                ++it1;
                ++it2;
            }
            ++it1;
            ++it2;
            continue;
        }

        // 如果一个是符号一个不是，符号排在后面
        if (isSymbol1 ^ isSymbol2)
            return isSymbol2;

        // 处理汉字(包括扩展汉字)
        QChar::Script script1 = it1->isHighSurrogate() ? QChar::script(unicode1) : it1->script();
        QChar::Script script2 = it2->isHighSurrogate() ? QChar::script(unicode2) : it2->script();
        isHanzi1 = script1 == QChar::Script_Han;
        isHanzi2 = script2 == QChar::Script_Han;

        if (isHanzi2 ^ isHanzi1)
            return !isHanzi1;
        if (isHanzi1) {
            // 直接使用 QString 构造包含单个 Unicode 码点的字符串
            QString str1 = makeQString(it1, unicode1);
            QString str2 = makeQString(it2, unicode2);
            return sortCollator.compare(str1, str2) < 0;
        }

        // 处理普通字符
        if (!isNumb1 && !isNumb2) {
            QString str1 = makeQString(it1, unicode1);
            QString str2 = makeQString(it2, unicode2);
            return str1.toLower() < str2.toLower();
        }

        return isNumb1;
    }

    // 如果前面的字符都相同，较短的字符串排在前面
    // (注意：这里需要检查两个迭代器是否都到达末尾)
    if (it1 == name1.constEnd() && it2 != name2.constEnd()) {
        return true;   // name1 is a prefix of name2
    }
    if (it1 != name1.constEnd() && it2 == name2.constEnd()) {
        return false;   // name2 is a prefix of name1
    }
    // 如果两个名字相同(it1 == name1.end() && it2 == name2.end())，则继续比较后缀

    // 处理后缀
    // 文件没有后缀名，排在有后缀名前面
    if (suf1.isEmpty() && !suf2.isEmpty()) return true;
    if (!suf1.isEmpty() && suf2.isEmpty()) return false;

    // 都有或都没有后缀，直接按后缀字典序比较
    return sortCollator.compare(suf1, suf2) < 0;
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
    return compareStringDefault(str1, str2);
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
    if (isFullWidthChar(ch, number))
        return isNumber(number);

    auto chValue = ch.unicode();
    return (chValue >= 48 && chValue <= 57);
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
    auto total = str.length();

    while (pos > 0 && isNumber(str.at(pos))) {
        pos--;
    }

    if (!isNumber(str.at(pos)))
        pos++;

    while (pos < total && isNumber(str.at(pos))) {
        const QChar &ch = str.at(pos);
        QChar number;
        if (isFullWidthChar(ch, number)) {
            tmp += number;   // 将全角数字转换为半角数字
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

QString makeQString(const QString::const_iterator &it, uint unicode)
{
    if (it->isHighSurrogate()) {
        QString str(QChar::highSurrogate(unicode));
        str.append(QChar::lowSurrogate(unicode));
        return str;
    }
    return *it;
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
        return compareStringDefault(str1, str2);
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

QString displayType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->displayTypeFromPath(path);
}

QString fastMimeType(const QUrl &url)
{
    const QString path = getLocalPath(url);
    return MimeTypeDisplayManager::instance()->fastMimeTypeName(path);
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
