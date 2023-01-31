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
#ifndef FILTERAPPENDER_H
#define FILTERAPPENDER_H

#include <dfm-framework/dfm_framework_global.h>

#include <DLog>

DPF_BEGIN_NAMESPACE

class FilterAppenderPrivate;
class FilterAppender : public DTK_CORE_NAMESPACE::FileAppender
{
    friend class FilterAppenderPrivate;

public:
    /*!
     * The enum DatePattern defines constants for date patterns.
     * \sa setDatePattern(DatePattern)
     */
    enum DatePattern {
        /*! The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
        kMinutelyRollover = 0,
        /*! The hourly date pattern string is "'.'yyyy-MM-dd-hh". */
        kHourlyRollover,
        /*! The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
        kHalfDailyRollover,
        /*! The daily date pattern string is "'.'yyyy-MM-dd". */
        kDailyRollover,
        /*! The weekly date pattern string is "'.'yyyy-ww". */
        kWeeklyRollover,
        /*! The monthly date pattern string is "'.'yyyy-MM". */
        kMonthlyRollover
    };

    explicit FilterAppender(const QString &fileName = QString());

    DatePattern datePattern() const;
    void setDatePattern(DatePattern datePattern);
    void setDatePattern(const QString &datePattern);

    QString datePatternString() const;
    void setLogFilesLimit(int limit);
    int logFilesLimit() const;

    void addFilter(const QString &filterField);
    void removeFilter(const QString &filterField);
    const QStringList &getFilters() const;
    void clearFilters();

protected:
    virtual void append(const QDateTime &timeStamp, DTK_CORE_NAMESPACE::Logger::LogLevel logLevel, const char *file, int line,
                        const char *function, const QString &category, const QString &message) override;

private:
    QSharedPointer<FilterAppenderPrivate> d;
};

DPF_END_NAMESPACE

#endif   // FILTERAPPENDER_H
