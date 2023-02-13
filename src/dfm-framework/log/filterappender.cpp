// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filterappender_p.h"

#include <QDir>
#include <QFileInfo>

DCORE_USE_NAMESPACE
DPF_USE_NAMESPACE

FilterAppenderPrivate::FilterAppenderPrivate(FilterAppender *qq)
    : frequency(FilterAppender::kMinutelyRollover),
      logFilesLimit(0),
      logSizeLimit(1024 * 1024 * 20),
      q(qq)
{
}

void FilterAppenderPrivate::rollOver()
{
    Q_ASSERT_X(!datePatternString.isEmpty(), "DailyRollingFileAppender::rollOver()", "No active date pattern");

    QString suffix = rollOverSuffix;
    computeRollOverTime();
    if (suffix == rollOverSuffix)
        return;

    q->closeFile();

    QString targetFileName = q->fileName() + suffix;
    QFile f(targetFileName);
    if (f.exists() && !f.remove())
        return;
    f.setFileName(q->fileName());
    if (!f.rename(targetFileName))
        return;

    q->openFile();
    removeOldFiles();
}

void FilterAppenderPrivate::computeRollOverTime()
{
    Q_ASSERT_X(!datePatternString.isEmpty(), "DailyRollingFileAppender::computeRollOverTime()", "No active date pattern");

    QDateTime now = QDateTime::currentDateTime();
    QDate nowDate = now.date();
    QTime nowTime = now.time();
    QDateTime start;

    switch (frequency) {
    case FilterAppender::kMinutelyRollover: {
        start = QDateTime(nowDate, nowTime);
        rollOverTime = start.addSecs(60);
    } break;
    case FilterAppender::kHourlyRollover: {
        start = QDateTime(nowDate, nowTime);
        rollOverTime = start.addSecs(60 * 60);
    } break;
    case FilterAppender::kHalfDailyRollover: {
        int hour = nowTime.hour();
        if (hour >= 12)
            hour = 12;
        else
            hour = 0;
        start = QDateTime(nowDate, nowTime);
        rollOverTime = start.addSecs(60 * 60 * 12);
    } break;
    case FilterAppender::kDailyRollover: {
        start = QDateTime(nowDate, nowTime);
        rollOverTime = start.addDays(1);
    } break;
    case FilterAppender::kWeeklyRollover: {
        // Qt numbers the week days 1..7. The week starts on Monday.
        // Change it to being numbered 0..6, starting with Sunday.
        int day = nowDate.dayOfWeek();
        if (day == Qt::Sunday)
            day = 0;
        start = QDateTime(nowDate, nowTime).addDays(-1 * day);
        rollOverTime = start.addDays(7);
    } break;
    case FilterAppender::kMonthlyRollover: {
        start = QDateTime(QDate(nowDate.year(), nowDate.month(), 1), nowTime);
        rollOverTime = start.addMonths(1);
    } break;
    }

    rollOverSuffix = start.toString(datePatternString);
    Q_ASSERT_X(rollOverSuffix != rollOverTime.toString(datePatternString),
               "DailyRollingFileAppender::computeRollOverTime()", "File name does not change with rollover");
}

void FilterAppenderPrivate::computeFrequency()
{
    QMutexLocker locker(&rollingMutex);

    const QDateTime startTime(QDate(1999, 1, 1), QTime(0, 0));
    const QString startString = startTime.toString(datePatternString);

    if (startString != startTime.addSecs(60).toString(datePatternString))
        frequency = FilterAppender::kMinutelyRollover;
    else if (startString != startTime.addSecs(60 * 60).toString(datePatternString))
        frequency = FilterAppender::kHourlyRollover;
    else if (startString != startTime.addSecs(60 * 60 * 12).toString(datePatternString))
        frequency = FilterAppender::kHalfDailyRollover;
    else if (startString != startTime.addDays(1).toString(datePatternString))
        frequency = FilterAppender::kDailyRollover;
    else if (startString != startTime.addDays(7).toString(datePatternString))
        frequency = FilterAppender::kWeeklyRollover;
    else if (startString != startTime.addMonths(1).toString(datePatternString))
        frequency = FilterAppender::kMonthlyRollover;
    else {
        Q_ASSERT_X(false, "DailyRollingFileAppender::computeFrequency", "The pattern '%1' does not specify a frequency");
        return;
    }
}

void FilterAppenderPrivate::removeOldFiles()
{
    if (logFilesLimit <= 1)
        return;

    QFileInfo fileInfo(q->fileName());
    QDir logDirectory(fileInfo.absoluteDir());
    logDirectory.setFilter(QDir::Files);
    logDirectory.setNameFilters(QStringList() << fileInfo.fileName() + "*");
    QFileInfoList logFiles = logDirectory.entryInfoList();

    QMap<QDateTime, QString> fileDates;
    for (int i = 0; i < logFiles.length(); ++i) {
        QString name = logFiles[i].fileName();
        QString suffix = name.mid(name.indexOf(fileInfo.fileName()) + fileInfo.fileName().length());
        QDateTime fileDateTime = QDateTime::fromString(suffix, q->datePatternString());

        if (fileDateTime.isValid())
            fileDates.insert(fileDateTime, logFiles[i].absoluteFilePath());
    }

    QList<QString> fileDateNames = fileDates.values();
    for (int i = 0; i < fileDateNames.length() - logFilesLimit + 1; ++i)
        QFile::remove(fileDateNames[i]);
}

void FilterAppenderPrivate::setDatePatternString(const QString &datePattern)
{
    QMutexLocker locker(&rollingMutex);
    datePatternString = datePattern;
}

/*!
 * \class FilterAppender
 * \brief The RollingFileAppender(modifed as FilterAppender) class extends FileAppender so that the underlying file is rolled over at a user chosen frequency.
 *
 * The class is based on Log4Qt.DailyRollingFileAppender class (http://log4qt.sourceforge.net/)
 * and has the same date pattern format.
 *
 * For example, if the fileName is set to /foo/bar and the DatePattern set to the daily rollover ('.'yyyy-MM-dd'.log'), on 2014-02-16 at midnight,
 * the logging file /foo/bar.log will be copied to /foo/bar.2014-02-16.log and logging for 2014-02-17 will continue in /foo/bar
 * until it rolls over the next day.
 *
 * The logFilesLimit parameter is used to automatically delete the oldest log files in the directory during rollover
 * (so no more than logFilesLimit recent log files exist in the directory at any moment).
 * \sa setDatePattern(DatePattern), setLogFilesLimit(int)
 */

FilterAppender::FilterAppender(const QString &fileName)
    : FileAppender(fileName),
      d(new FilterAppenderPrivate(this))
{
}

void FilterAppender::append(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line,
                            const char *function, const QString &category, const QString &message)
{
    QMutexLocker locker(&d->filterMutex);

    //! filter key words
    for (const auto &filter : d->keyFilters) {
        if (message.contains(filter))
            return;
    }
    locker.unlock();

    if (!d->rollOverTime.isNull() && QDateTime::currentDateTime() > d->rollOverTime)
        d->rollOver();

    if (size() > d->logSizeLimit)
        d->rollOver();

    FileAppender::append(timeStamp, logLevel, file, line, function, category, message);
}

FilterAppender::DatePattern FilterAppender::datePattern() const
{
    QMutexLocker locker(&d->rollingMutex);
    return d->frequency;
}

QString FilterAppender::datePatternString() const
{
    QMutexLocker locker(&d->rollingMutex);
    return d->datePatternString;
}

void FilterAppender::setDatePattern(DatePattern datePattern)
{
    d->setDatePatternString(QLatin1String("'.'yyyy-MM-dd-hh-mm-zzz"));

    QMutexLocker locker(&d->rollingMutex);
    d->frequency = datePattern;

    d->computeRollOverTime();
}

void FilterAppender::setDatePattern(const QString &datePattern)
{
    d->setDatePatternString(datePattern);
    d->computeFrequency();

    d->computeRollOverTime();
}

void FilterAppender::setLogFilesLimit(int limit)
{
    QMutexLocker locker(&d->rollingMutex);
    d->logFilesLimit = limit;
}

int FilterAppender::logFilesLimit() const
{
    QMutexLocker locker(&d->rollingMutex);
    return d->logFilesLimit;
}

void FilterAppender::addFilter(const QString &filterField)
{
    QMutexLocker locker(&d->filterMutex);
    d->keyFilters << filterField;
}

void FilterAppender::removeFilter(const QString &filterField)
{
    QMutexLocker locker(&d->filterMutex);
    d->keyFilters.removeAll(filterField);
}

const QStringList &FilterAppender::getFilters() const
{
    return d->keyFilters;
}

void FilterAppender::clearFilters()
{
    QMutexLocker locker(&d->filterMutex);
    d->keyFilters.clear();
}
