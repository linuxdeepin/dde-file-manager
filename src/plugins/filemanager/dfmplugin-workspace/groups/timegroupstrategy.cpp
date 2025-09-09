// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

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
        "past-30-days",   // 过去30天
        "this-year",   // 今年其他月份
        "past-years",   // 过去年份
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
    fmDebug() << "TimeGroupStrategy: Initialized with time type:" << (timeType == ModificationTime ? "Modification" : "Creation");
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

    // Get the appropriate timestamp based on the time type
    QDateTime fileTime;
    if (m_timeType == ModificationTime) {
        fileTime = info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
    } else {
        fileTime = info->timeOf(TimeInfoType::kCreateTime).value<QDateTime>();
    }

    if (!fileTime.isValid()) {
        fmWarning() << "TimeGroupStrategy: Invalid file time for" << info->urlOf(UrlInfoType::kUrl).toString();
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

QStringList TimeGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    // For time grouping, we need to handle dynamic groups
    // Start with the basic time order
    QStringList result = getTimeOrder();

    // Note: Dynamic groups (months, years) will be inserted at runtime
    // based on the actual files present

    if (order == Qt::DescendingOrder) {
        std::reverse(result.begin(), result.end());
    }

    return result;
}

int TimeGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    // Handle dynamic time groups
    if (groupKey.startsWith("month-") || groupKey.startsWith("year-")) {
        return getDynamicDisplayOrder(groupKey, order);
    }

    // Handle basic time groups
    QStringList timeOrder = getTimeOrder();
    int index = timeOrder.indexOf(groupKey);
    if (index == -1) {
        index = timeOrder.size();   // Unknown groups go to the end
    }

    if (order == Qt::AscendingOrder) {
        return index;
    } else {
        return timeOrder.size() - index - 1;
    }
}

bool TimeGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one file info
    return !infos.isEmpty();
}

QString TimeGroupStrategy::getStrategyName() const
{
    if (m_timeType == ModificationTime) {
        return tr("Modified Time");
    } else {
        return tr("Created Time");
    }
}

Global::ItemRoles TimeGroupStrategy::getCorrespondingRole() const
{
    if (m_timeType == ModificationTime) {
        return Global::ItemRoles::kItemFileLastModifiedRole;
    } else {
        return Global::ItemRoles::kItemFileCreatedRole;
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

    // Today
    if (fileDate == today) {
        return "today";
    }

    // Yesterday
    if (fileDate == today.addDays(-1)) {
        return "yesterday";
    }

    qint64 daysAgo = fileDate.daysTo(today);

    // Past 7 days (excluding today and yesterday)
    if (daysAgo <= 7) {
        return "past-7-days";
    }

    // Past 30 days (excluding past 7 days)
    if (daysAgo <= 30) {
        return "past-30-days";
    }

    // This year - other months
    if (fileDate.year() == today.year()) {
        return QString("month-%1").arg(fileDate.month());
    }

    int yearDiff = today.year() - fileDate.year();

    // Past 5 years
    if (yearDiff <= 5) {
        return QString("year-%1").arg(fileDate.year());
    }

    // Earlier (more than 5 years ago)
    return "earlier";
}

int TimeGroupStrategy::getDynamicDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    QDate today = QDate::currentDate();

    if (groupKey.startsWith("month-")) {
        bool ok;
        int month = groupKey.mid(6).toInt(&ok);
        if (!ok) return 1000;   // Invalid, put at end

        // Months are ordered by recency (current month = 0, previous month = 1, etc.)
        int monthsAgo = (today.year() - today.year()) * 12 + (today.month() - month);
        if (monthsAgo < 0) monthsAgo += 12;   // Handle year boundary

        // Month groups come after "past-30-days" (index 3) but before "past-years"
        int baseIndex = 4;   // After "past-30-days"

        if (order == Qt::AscendingOrder) {
            return baseIndex + monthsAgo;
        } else {
            return baseIndex - monthsAgo;
        }

    } else if (groupKey.startsWith("year-")) {
        bool ok;
        int year = groupKey.mid(5).toInt(&ok);
        if (!ok) return 1000;   // Invalid, put at end

        // Years are ordered by recency (current year = 0, previous year = 1, etc.)
        int yearsAgo = today.year() - year;

        // Year groups come after month groups but before "earlier"
        int baseIndex = 100;   // After all possible month groups

        if (order == Qt::AscendingOrder) {
            return baseIndex + yearsAgo;
        } else {
            return baseIndex - yearsAgo;
        }
    }

    return 1000;   // Unknown, put at end
}
