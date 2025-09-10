// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "namegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/chinese2pinyin.h>

#include <QDebug>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QStringList NameGroupStrategy::getNameOrder()
{
    return {
        "0-9",   // 数字
        "A-H",   // 英文 A-H
        "I-P",   // 英文 I-P
        "Q-Z",   // 英文 Q-Z
        "pinyin-A-H",   // 拼音 A-H
        "pinyin-I-P",   // 拼音 I-P
        "pinyin-Q-Z",   // 拼音 Q-Z
        "others"   // 其他
    };
}

QHash<QString, QString> NameGroupStrategy::getDisplayNames()
{
    return {
        { "0-9", QObject::tr("0-9") },
        { "A-H", QObject::tr("A-H") },
        { "I-P", QObject::tr("I-P") },
        { "Q-Z", QObject::tr("Q-Z") },
        { "pinyin-A-H", QObject::tr("Chinese A-H") },
        { "pinyin-I-P", QObject::tr("Chinese I-P") },
        { "pinyin-Q-Z", QObject::tr("Chinese Q-Z") },
        { "others", QObject::tr("Others") }
    };
}

NameGroupStrategy::NameGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "NameGroupStrategy: Initialized";
}

NameGroupStrategy::~NameGroupStrategy()
{
    fmDebug() << "NameGroupStrategy: Destroyed";
}

QString NameGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    if (!info) {
        fmWarning() << "NameGroupStrategy: Invalid fileInfo";
        return "others";
    }

    QString name = info->displayOf(DisPlayInfoType::kFileDisplayName);

    if (name.isEmpty()) {
        fmWarning() << "NameGroupStrategy: Empty file name for" << info->urlOf(UrlInfoType::kUrl).toString();
        return "others";
    }

    QString groupKey = classifyFirstCharacter(name.at(0));

    fmDebug() << "NameGroupStrategy: File" << info->urlOf(UrlInfoType::kUrl).toString()
              << "name:" << name << "first char:" << name.at(0) << "-> group:" << groupKey;

    return groupKey;
}

QString NameGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    return getDisplayNames().value(groupKey, groupKey);
}

QStringList NameGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    QStringList nameOrder = getNameOrder();
    if (order == Qt::AscendingOrder) {
        return nameOrder;
    } else {
        QStringList reversed = nameOrder;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
}

int NameGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    QStringList nameOrder = getNameOrder();
    int index = nameOrder.indexOf(groupKey);
    if (index == -1) {
        index = nameOrder.size();   // Unknown groups go to the end
    }

    if (order == Qt::AscendingOrder) {
        return index;
    } else {
        return nameOrder.size() - index - 1;
    }
}

bool NameGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one file info
    return !infos.isEmpty();
}

QString NameGroupStrategy::getStrategyName() const
{
    return GroupStrategty::kName;
}

QString NameGroupStrategy::classifyFirstCharacter(const QChar &ch) const
{
    // Check for digits
    if (ch.isDigit()) {
        return "0-9";
    }

    // Check for English letters
    if (ch.isLetter() && ch.unicode() < 128) {
        char upper = ch.toUpper().toLatin1();
        if (upper >= 'A' && upper <= 'H') {
            return "A-H";
        } else if (upper >= 'I' && upper <= 'P') {
            return "I-P";
        } else if (upper >= 'Q' && upper <= 'Z') {
            return "Q-Z";
        }
    }

    // Check for Chinese characters
    if (isChinese(ch)) {
        QString pinyin = getPinyin(ch);
        if (!pinyin.isEmpty()) {
            char first = pinyin.at(0).toUpper().toLatin1();
            if (first >= 'A' && first <= 'H') {
                return "pinyin-A-H";
            } else if (first >= 'I' && first <= 'P') {
                return "pinyin-I-P";
            } else if (first >= 'Q' && first <= 'Z') {
                return "pinyin-Q-Z";
            }
        }
    }

    // Everything else goes to "others"
    return "others";
}

bool NameGroupStrategy::isChinese(const QChar &ch) const
{
    ushort unicode = ch.unicode();
    // Check CJK Unified Ideographs and CJK Extension A ranges
    return (unicode >= 0x4E00 && unicode <= 0x9FFF) ||   // CJK Unified Ideographs
            (unicode >= 0x3400 && unicode <= 0x4DBF);   // CJK Extension A
}

QString NameGroupStrategy::getPinyin(const QChar &ch) const
{
    // Use the existing Pinyin::Chinese2Pinyin function
    QString singleChar = QString(ch);
    QString pinyinResult = Pinyin::Chinese2Pinyin(singleChar);

    if (!pinyinResult.isEmpty()) {
        // The function returns a space-separated string, take the first word
        QStringList pinyinWords = pinyinResult.split(' ', Qt::SkipEmptyParts);
        if (!pinyinWords.isEmpty() && !pinyinWords.first().isEmpty()) {
            return pinyinWords.first();
        }
    }

    fmDebug() << "NameGroupStrategy: Failed to convert Chinese character to pinyin:" << ch;
    return QString();   // Conversion failed
}
