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
#include "private/framelogmanager_p.h"
#include "filterappender.h"

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
        logger->registerAppender(curFilterAppender);
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
