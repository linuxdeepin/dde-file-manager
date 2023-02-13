// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
