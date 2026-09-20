// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinyinprocessor.h"

#include <QFile>
#include <QList>
#include <QStandardPaths>
#include <QTextStream>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

// 字典安装路径由 CMake 注入（PINYIN_DICT_INSTALL_DIR 与 install(FILES resources/pinyin.txt)
// 的 DESTINATION 同源，见 textindex/CMakeLists.txt）；此处仅为无编译定义时（IDE 解析、
// 单独编译）的回退值，须与 debian 打包路径一致。
#ifndef PINYIN_DICT_INSTALL_DIR
#    define PINYIN_DICT_INSTALL_DIR "/usr/share/deepin/dde-file-manager"
#endif

constexpr char kDefaultPinyinDictPath[] { PINYIN_DICT_INSTALL_DIR "/pinyin.txt" };

// 码点 → 字典键（UTF-16 序列）：BMP 内为 1 个单元，CJK 扩展区等补充平面字符
// 为 surrogate pair（2 个单元）。不可直接用码点构造 QChar（>0xFFFF 会触发断言）。
QString codePointKey(uint codePoint)
{
    if (QChar::requiresSurrogates(codePoint)) {
        QString key;
        key += QChar(QChar::highSurrogate(codePoint));
        key += QChar(QChar::lowSurrogate(codePoint));
        return key;
    }
    return QString(QChar(codePoint));
}

}   // namespace

PinyinProcessor &PinyinProcessor::instance()
{
    static PinyinProcessor instance;
    return instance;
}

QStringList PinyinProcessor::dictionaryLookupPaths()
{
    // QStandardPaths::writableLocation(GenericDataLocation) 返回 XDG_DATA_HOME
    // （未设置时默认 ~/.local/share）
    return {
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                + QStringLiteral("/deepin/dde-file-manager/pinyin.txt"),
        QString::fromLatin1(kDefaultPinyinDictPath)
    };
}

PinyinProcessor::PinyinProcessor()
{
    for (const QString &path : dictionaryLookupPaths()) {
        if (loadDictionary(path)) {
            fmInfo() << "PinyinProcessor: Dictionary loaded from:" << path;
            break;
        }
    }
}

bool PinyinProcessor::loadDictionary(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmWarning() << "PinyinProcessor: Failed to open pinyin dictionary:" << path;
        m_loaded = false;
        return false;
    }

    // 字典格式（与 deepin-anything 一致）：
    //   U+3007: líng,yuán,xīng  # 〇
    // 注释行以 '#' 开头；Unicode 码点（hex）→ 拼音列表（逗号分隔，含声调）；'#' 后为汉字注释。
    m_pinyinMap.clear();   // 支持重新加载（测试和配置切换场景）
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.size() < 2)
            continue;
        if (line[0] != 'U' && line[0] != 'u')
            continue;
        if (line[1] != '+')
            continue;

        const int colonPos = line.indexOf(':');
        if (colonPos < 3)
            continue;

        // 解析 Unicode 码点（十六进制）
        bool ok = false;
        const uint codePoint = line.mid(2, colonPos - 2).toUInt(&ok, 16);
        if (!ok || codePoint == 0)
            continue;
        // 防御非法/超范围码点（字典可被用户目录覆盖，内容不受控）：
        // 超出 Unicode 上限的行跳过，避免后续构造越界
        if (codePoint > QChar::LastValidCodePoint)
            continue;

        // 取冒号后、'#' 前的拼音部分（'#' 可能不存在）
        const int hashPos = line.indexOf('#', colonPos);
        const QString pinyinStr = (hashPos > colonPos
                                           ? line.mid(colonPos + 1, hashPos - colonPos - 1)
                                           : line.mid(colonPos + 1))
                                          .trimmed();
        if (pinyinStr.isEmpty())
            continue;

        // 多音字取第一个（与 anything 的 it->second[0] 一致）
        const QStringList pys = pinyinStr.split(',', Qt::SkipEmptyParts);
        if (pys.isEmpty())
            continue;

        const QString first = removeTone(pys.first().trimmed());
        if (!first.isEmpty())
            m_pinyinMap.insert(codePointKey(codePoint), first);
    }

    file.close();
    m_loaded = !m_pinyinMap.isEmpty();
    fmInfo() << "PinyinProcessor: Loaded" << m_pinyinMap.size() << "pinyin entries from" << path;
    return m_loaded;
}

bool PinyinProcessor::isLoaded() const
{
    return m_loaded;
}

void PinyinProcessor::convertToPinyin(const QString &fileName,
                                      QString &pinyinFull,
                                      QString &pinyinAcronym) const
{
    pinyinFull.clear();
    pinyinAcronym.clear();

    if (!m_loaded || fileName.isEmpty())
        return;

    pinyinFull.reserve(fileName.size() * 6);
    pinyinAcronym.reserve(fileName.size());

    // 按码点（UTF-32）遍历：QChar 逐单元遍历会把补充平面字符（CJK 扩展区汉字）
    // 拆成孤立代理项，无法命中字典。toUcs4 对无效代理序列替换为 U+FFFD，安全。
    const QList<uint> codePoints = fileName.toUcs4();
    for (const uint codePoint : codePoints) {
        const QString key = codePointKey(codePoint);
        auto it = m_pinyinMap.constFind(key);
        if (it != m_pinyinMap.constEnd()) {
            // 汉字：拼接去声调后的拼音
            pinyinFull += it.value();
            pinyinAcronym += it.value().at(0);
        } else {
            // 非字典字符（英文/数字/符号）：原样拼入，保持 anything 行为
            pinyinFull += key;
            pinyinAcronym += key;
        }
    }

    // 整体小写化（anything 写入索引时 toLower）
    pinyinFull = pinyinFull.toLower();
    pinyinAcronym = pinyinAcronym.toLower();
}

QString PinyinProcessor::removeTone(const QString &pinyin) const
{
    // 与 anything remove_tone 的 tone_map 完全一致
    static const QHash<QChar, QChar> toneMap = {
        { u'ā', u'a' }, { u'á', u'a' }, { u'ǎ', u'a' }, { u'à', u'a' }, { u'ē', u'e' }, { u'é', u'e' }, { u'ě', u'e' }, { u'è', u'e' }, { u'ī', u'i' }, { u'í', u'i' }, { u'ǐ', u'i' }, { u'ì', u'i' }, { u'ō', u'o' }, { u'ó', u'o' }, { u'ǒ', u'o' }, { u'ò', u'o' }, { u'ū', u'u' }, { u'ú', u'u' }, { u'ǔ', u'u' }, { u'ù', u'u' }, { u'ǖ', u'v' }, { u'ǘ', u'v' }, { u'ǚ', u'v' }, { u'ǜ', u'v' }, { u'ü', u'v' }
    };

    QString result;
    result.reserve(pinyin.size());
    for (const QChar &ch : pinyin) {
        auto it = toneMap.constFind(ch);
        result += (it != toneMap.constEnd()) ? it.value() : ch;
    }
    return result;
}

SERVICETEXTINDEX_END_NAMESPACE
