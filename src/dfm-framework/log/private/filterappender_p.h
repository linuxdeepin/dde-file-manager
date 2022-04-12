/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILTERAPPENDER_P_H
#define FILTERAPPENDER_P_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/log/filterappender.h"

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
