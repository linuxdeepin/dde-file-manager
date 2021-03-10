/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "dfmLogManager.h"
#include <Logger.h>
#include <ConsoleAppender.h>
#include "filterAppender.h"
#include <LogManager.h>
#include <Logger.h>


DCORE_USE_NAMESPACE
/**
 * \class DFMLogManager
 *
 * \brief DFMLogManager is the deepin user application log manager
 */

DFM_BEGIN_NAMESPACE
DFMLogManager::DFMLogManager()
    : m_filterAppender(nullptr)
{
}

void DFMLogManager::initFilterAppender()
{
    if (m_filterAppender)
        return;

    m_filterAppender = new FilterAppender(DLogManager::getlogFilePath());
    m_filterAppender->setFormat(
                "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n");
    m_filterAppender->setLogFilesLimit(5);
    m_filterAppender->setDatePattern(FilterAppender::DailyRollover);
    logger->registerAppender(m_filterAppender);
}

FilterAppender *DFMLogManager::filterAppender()
{
    if (!m_filterAppender) {
        initFilterAppender();
    }
    return m_filterAppender;
}

//! Registers the appender to write the log records to the Console
/**
 * \sa registerFileAppender
 */
void DFMLogManager::registerConsoleAppender()
{
    DLogManager::registerConsoleAppender();
}

//! Registers the appender to write the log records to the file
/**
 * \sa getlogFilePath
 * \sa registerConsoleAppender
 */
void DFMLogManager::registerFileAppender()
{
    DFMLogManager::instance()->initFilterAppender();
}

//! Return the path file log storage
/**
 * \sa registerFileAppender
 */
QString DFMLogManager::getlogFilePath()
{
    return DLogManager::getlogFilePath();
}

void DFMLogManager::setlogFilePath(const QString &logFilePath)
{
    DLogManager::setlogFilePath(logFilePath);
}

void DFMLogManager::setLogFormat(const QString &format)
{
    DLogManager::setLogFormat(format);
}

Logger *DFMLogManager::getLogger()
{
    return Logger::globalInstance();
}

FilterAppender *DFMLogManager::getFilterAppender()
{
    return DFMLogManager::instance()->filterAppender();
}

DFMLogManager::~DFMLogManager()
{
}
DFM_END_NAMESPACE
