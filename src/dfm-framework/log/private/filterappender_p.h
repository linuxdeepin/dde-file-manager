// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILTERAPPENDER_P_H
#define FILTERAPPENDER_P_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/log/filterappender.h>

#include <QDateTime>

DPF_BEGIN_NAMESPACE

class FilterAppenderPrivate
{
public:
    explicit FilterAppenderPrivate(FilterAppender *qq);

    void rollOver();
    void computeRollOverTime();
    void computeFrequency();
    void removeOldFiles();
    void setDatePatternString(const QString &datePattern);

public:
    QString datePatternString;
    FilterAppender::DatePattern frequency;

    QDateTime rollOverTime;
    QString rollOverSuffix;
    int logFilesLimit;
    qint64 logSizeLimit;
    mutable QMutex rollingMutex;

    QStringList keyFilters;
    mutable QMutex filterMutex;

    FilterAppender *const q;
};

DPF_END_NAMESPACE

#endif   // FILTERAPPENDER_P_H
