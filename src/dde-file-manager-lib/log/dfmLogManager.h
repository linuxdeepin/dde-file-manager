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

#ifndef DFMLOGMANAGER_H
#define DFMLOGMANAGER_H

#include <QtCore>
#include <dfmglobal.h>

class ConsoleAppender;
class FilterAppender;
namespace Dtk {
    namespace Core {
        class Logger;
    }
}

DFM_BEGIN_NAMESPACE
/**
 * @brief The DFMLogManager class
 * re-wrap the log function of dtk, add filter function.
 */
class DFMLogManager
{
public:
    static void registerConsoleAppender();
    static void registerFileAppender();

    static QString getlogFilePath();

    /*!
     * \brief setlogFilePath will change log file path of registerFileAppender
     * \param logFilePath is the full path of file appender log
     */
    static void setlogFilePath(const QString& logFilePath);

    static void setLogFormat(const QString& format);

    static Dtk::Core::Logger *getLogger();

    static FilterAppender *getFilterAppender();

private:
    FilterAppender* m_filterAppender;

    void initFilterAppender();
    FilterAppender *filterAppender();

    inline static DFMLogManager* instance(){
        static DFMLogManager instance;
        return &instance;
    }
    explicit DFMLogManager();
    ~DFMLogManager();
    DFMLogManager(const DFMLogManager &);
    DFMLogManager & operator = (const DFMLogManager &);
};
DFM_END_NAMESPACE

#endif // DFMLOGMANAGER_H
