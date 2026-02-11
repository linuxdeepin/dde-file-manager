// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/protocolutils.h>

#include <QDate>
#include <QLocale>
#include <QDebug>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QStringList TimeGroupStrategy::getTimeOrder()
{
    return {
        "today",   // 今天
        "yesterday",   // 昨天
        "past-7-days",   // 过去7天
        "past-30-days",   // 过去30天s
        "earlier"   // 更早
    };
}

QHash<QString, QString> TimeGroupStrategy::getDisplayNames()
{
    return {
        { "today", QObject::tr("Today") },
        { "yesterday", QObject::tr("Yesterday") },
        { "past-7-days", QObject::tr("Past 7 Days") },
        { "past-30-days", QObject::tr("Past 30 Days") },
        { "earlier", QObject::tr("Earlier") }
    };
}

TimeGroupStrategy::TimeGroupStrategy(TimeType timeType, QObject *parent)
    : AbstractGroupStrategy(parent), m_timeType(timeType)
{
    fmDebug() << "TimeGroupStrategy: Initialized with time type:" << (timeType == kModificationTime ? "Modification" : "Creation");
}

TimeGroupStrategy::~TimeGroupStrategy()
{
    fmDebug() << "TimeGroupStrategy: Destroyed";
}

QString TimeGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    if (!info) {
        fmWarning() << "TimeGroupStrategy: Invalid fileInfo";
        return "earlier";
    }

    // AsyncFileinfo 的设计缺陷无法实时获取到时间相关属性
    FileInfoPointer newFileInfo { info };
    const auto url { info->urlOf(UrlInfoType::kUrl) };
    if (ProtocolUtils::isRemoteFile(url)) {
        newFileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    }

    // Get the appropriate timestamp based on the time type
    QDateTime fileTime;
    if (m_timeType == kModificationTime) {
        fileTime = newFileInfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
    } else if (m_timeType == kCreationTime) {
        fileTime = newFileInfo->timeOf(TimeInfoType::kCreateTime).value<QDateTime>();
    } else {
        fileTime = newFileInfo->timeOf(TimeInfoType::kCustomerSupport).value<QDateTime>();
    }

    if (!fileTime.isValid()) {
        fmWarning() << "TimeGroupStrategy: Invalid file time for" << newFileInfo->urlOf(UrlInfoType::kUrl).toString();
        return "earlier";
    }

    QString groupKey = calculateTimeGroup(fileTime);

    fmDebug() << "TimeGroupStrategy: File" << info->urlOf(UrlInfoType::kUrl).toString()
              << "time:" << fileTime.toString() << "-> group:" << groupKey;

    return groupKey;
}

QString TimeGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    // Handle dynamic time display names
    if (groupKey.startsWith("month-")) {
        bool ok;
        int month = groupKey.mid(6).toInt(&ok);
        if (ok && month >= 1 && month <= 12) {
            QDate date(QDate::currentDate().year(), month, 1);
            return QLocale().monthName(month, QLocale::LongFormat);
        }
    } else if (groupKey.startsWith("year-")) {
        bool ok;
        int year = groupKey.mid(5).toInt(&ok);
        if (ok) {
            return QString::number(year);
        }
    }

    // Return static display names
    return getDisplayNames().value(groupKey, groupKey);
}

QStringList TimeGroupStrategy::getGroupOrder() const
{
    // For time grouping, we need to handle dynamic groups
    // Start with the basic time order
    QStringList result = getTimeOrder();

    // Note: Dynamic groups (months, years) will be inserted at runtime
    // based on the actual files present

    return result;
}

int TimeGroupStrategy::getGroupDisplayOrder(const QString &groupKey) const
{
    // 使用基数来确保大的分组类别顺序正确
    // 今天 (0) -> 昨天 (1) -> 过去7天 (2) -> 过去30天 (3)
    // -> 月份 (100+) -> 年份 (200+) -> 更早 (9999)

    if (groupKey == "today") return 0;
    if (groupKey == "yesterday") return 1;
    if (groupKey == "past-7-days") return 2;
    if (groupKey == "past-30-days") return 3;

    // 月份分组 (基数 100)
    // 越近的月份，排序值越小
    if (groupKey.startsWith("month-")) {
        bool ok;
        int month = groupKey.mid(6).toInt(&ok);
        if (ok) {
            QDate today = QDate::currentDate();
            int monthsAgo = today.month() - month;   // 在同一年内，这个差值越大说明月份越早
            return 100 + monthsAgo;   // 例如：5月文件(5-5=0)->100, 4月文件(5-4=1)->101
        }
    }

    // 年份分组 (基数 200)
    // 越近的年份，排序值越小
    if (groupKey.startsWith("year-")) {
        bool ok;
        int year = groupKey.mid(5).toInt(&ok);
        if (ok) {
            QDate today = QDate::currentDate();
            int yearsAgo = today.year() - year;
            return 200 + yearsAgo;   // 例如：2024年(2025-2024=1)->201, 2023年(2025-2023=2)->202
        }
    }

    // “更早” 永远是最后一个
    if (groupKey == "earlier") {
        return 9999;
    }

    // 未知分组也放在最后
    return 10000;
}

bool TimeGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one file info
    return !infos.isEmpty();
}

QString TimeGroupStrategy::getStrategyName() const
{
    if (m_timeType == kModificationTime) {
        return GroupStrategy::kModifiedTime;
    } else if (m_timeType == kCreationTime) {
        return GroupStrategy::kCreatedTime;
    } else {
        return GroupStrategy::kCustomTime;
    }
}

QString TimeGroupStrategy::calculateTimeGroup(const QDateTime &fileTime) const
{
    if (!fileTime.isValid()) {
        return "earlier";
    }

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QDate fileDate = fileTime.date();

    // 今天：时间为当日 00：00-23：59的文件。
    if (fileDate == today) {
        return "today";
    }

    // 昨天：时间为昨天 00：00-23：59的文件。
    if (fileDate == today.addDays(-1)) {
        return "yesterday";
    }

    // 过去 7天：按当前精确时分倒推 7*24h，排除今天和昨天。
    if (fileTime >= now.addDays(-7)) {
        return "past-7-days";
    }

    // 过去 30天：按当前精确时分倒推 30*24h，排除已分组的。
    if (fileTime >= now.addDays(-30)) {
        return "past-30-days";
    }

    // 月份：今年内的文件，排除以上所有。
    if (fileDate.year() == today.year()) {
        return QString("month-%1").arg(fileDate.month());
    }

    // 年份：最多显示过去 5年。
    // 例如今年是 2025年，显示 2024, 2023, 2022, 2021, 2020 年。
    int yearDiff = today.year() - fileDate.year();
    if (yearDiff >= 1 && yearDiff <= 5) {
        return QString("year-%1").arg(fileDate.year());
    }

    // 更早：5年以前的文件。
    // 例如今年是 2025年，2019年及之前的文件。
    return "earlier";
}
