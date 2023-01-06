// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
