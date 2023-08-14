// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/framelogmanager_p.h"

#include <dfm-framework/log/filterappender.h>

#include <mutex>

DCORE_USE_NAMESPACE
DPF_USE_NAMESPACE

FrameLogManagerPrivate::FrameLogManagerPrivate(FrameLogManager *qq)
    : q(qq)
{
}

void FrameLogManagerPrivate::initFilterAppender()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        curFilterAppender = new FilterAppender(DTK_CORE_NAMESPACE::DLogManager::getlogFilePath());
        curFilterAppender->setFormat(
                "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n");
        curFilterAppender->setLogFilesLimit(5);
        curFilterAppender->setDatePattern(FilterAppender::kDailyRollover);
        loggerInstance()->registerAppender(curFilterAppender);
    });
}

FilterAppender *FrameLogManagerPrivate::filterAppender()
{
    if (!curFilterAppender)
        initFilterAppender();
    return curFilterAppender;
}

/*!
 * \class The FrameLogManager class
 * re-wrap the log function of dtk, add filter function
 */

FrameLogManager *FrameLogManager::instance()
{
    static FrameLogManager ins;
    return &ins;
}

/*!
 * \brief Registers the appender to write the log records to the Console
 */
void FrameLogManager::registerConsoleAppender()
{
    DTK_CORE_NAMESPACE::DLogManager::registerConsoleAppender();
}

/*!
 * \brief Registers the appender to write the log records to the file
 */
void FrameLogManager::registerFileAppender()
{
    d->initFilterAppender();
}

/*!
 * \brief Return the path file log storage
 * \return path file log storage
 */
QString FrameLogManager::logFilePath()
{
    return DTK_CORE_NAMESPACE::DLogManager::getlogFilePath();
}

/*!
 * \brief setlogFilePath will change log file path of registerFileAppender
 * \param logFilePath is the full path of file appender log
 */
void FrameLogManager::setlogFilePath(const QString &logFilePath)
{
    DTK_CORE_NAMESPACE::DLogManager::setlogFilePath(logFilePath);
}

void FrameLogManager::setLogFormat(const QString &format)
{
    DTK_CORE_NAMESPACE::DLogManager::setLogFormat(format);
}

Dtk::Core::Logger *FrameLogManager::dtkLogger()
{
    return DTK_CORE_NAMESPACE::Logger::globalInstance();
}

FilterAppender *FrameLogManager::filterAppender()
{
    return d->filterAppender();
}

FrameLogManager::FrameLogManager(QObject *parent)
    : QObject(parent),
      d(new FrameLogManagerPrivate(this))
{
}

FrameLogManager::~FrameLogManager()
{
}
