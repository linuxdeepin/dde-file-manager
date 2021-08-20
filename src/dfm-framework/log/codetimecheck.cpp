#include "codetimecheck.h"

#include "dfm-framework/log/logutils.h"

//全局模块使能宏
#ifndef DPF_NO_CHECK_TIME

#include <QString>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QMutex>
#include <QDateTime>
#include <QDate>
#include <QDir>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {

    static QFile file;
    static uint dayCount;

#ifdef QT_NO_DEBUG
    const QString tcDirName = "codeTimeCheck";
    const QString tcFileName = "tc_release.csv";
#else
    const QString tcDirName = "codeTimeCheck";
    const QString tcFileName = "tc_debug.csv";
#endif

    static void rmExpiredLogs()
    {
        QtConcurrent::run([=](){
            QDirIterator itera(LogUtils::appCacheLogPath()
                               + "/" + tcDirName);
            while(itera.hasNext()) {
                itera.next();
                auto list = itera.fileName().split("_");
                if (itera.fileInfo().suffix() == "csv"
                        && list.count() == 3
                        && !LogUtils::containLastDay(
                            QDateTime(QDate::fromString(list[0],"yyyy-MM-dd"),
                                      QTime(0,0,0,0)),
                            LogUtils::toDayZero(),
                            GlobalPrivate::dayCount))
                {
                    qInfo("remove true(%d) not last week log: %s",
                          QDir().remove(itera.path() + "/" + itera.fileName()),
                          itera.fileName().toLocal8Bit().data());
                }
            }
        });
    }

    static void outCheck(const QMessageLogContext &context, const QString &msg)
    {

        QString currAppLogName = LogUtils::appCacheLogPath()
                + "/" + tcDirName + "/"
                + LogUtils::localDate() + "_" + tcFileName;

        // "codeTimeCheck" dir
        LogUtils::checkAppCacheLogDir(tcDirName);

        // 文件名称为空 或者当前日期不正确(跨天日志分割)
        if (file.fileName().isEmpty()
                || file.fileName() != currAppLogName) {
            file.setFileName(currAppLogName);
            qInfo() << "Current checkTime file path: " << file.fileName();
            rmExpiredLogs();
        }

        if(!file.isOpen()) {
            file.open(QFile::WriteOnly|QFile::Append);
        }

        auto fileNameList = QString(context.file).split("/");
        auto currentName = fileNameList[fileNameList.size() - 1];

        file.write((LogUtils::localDataTimeCSV() + ","
                    + context.function + ","
                    + msg + ","
                    + currentName
                    + "\n").toUtf8().data());
        file.flush();
        file.close();
    }

}//namespace GlobalPrivate

void CodeCheckTime::setLogCacheDayCount(uint dayCount)
{
    static QMutex mutex;
    mutex.lock();
    GlobalPrivate::dayCount = dayCount;
    mutex.unlock();
}

uint CodeCheckTime::logCacheDayCount()
{
    return GlobalPrivate::dayCount;
}

void CodeCheckTime::begin(const QMessageLogContext &context)
{
    static QMutex mutex;
    mutex.lock();
    GlobalPrivate::outCheck(context,"begin");
    mutex.unlock();
}

void CodeCheckTime::end(const QMessageLogContext &context)
{
    static QMutex mutex;
    mutex.lock();
    GlobalPrivate::outCheck(context,"end");
    mutex.unlock();
}

#endif // DPF_NO_CHECK_TIME

DPF_END_NAMESPACE
