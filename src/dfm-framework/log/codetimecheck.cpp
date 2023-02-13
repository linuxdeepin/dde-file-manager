// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/log/codetimecheck.h>
#include <dfm-framework/log/logutils.h>

//全局模块使能宏
#ifndef DPF_NO_CHECK_TIME

#    include <QString>
#    include <QFile>
#    include <QCoreApplication>
#    include <QStandardPaths>
#    include <QDebug>
#    include <QMutex>
#    include <QDateTime>
#    include <QDate>
#    include <QDir>
#    include <QtConcurrent>
#    include <unistd.h>

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {

static uint kDayCount = 7;

#    ifdef QT_NO_DEBUG
static const char *tcDirName = "codeTimeCheck";
static const char *tcFileName = "tc_release.csv";
#    else
static const char *tcDirName = "codeTimeCheck";
static const char *tcFileName = "tc_debug.csv";
#    endif

static QFile *file()
{
    static QFile file;
    return &file;
}

static QMutex *mutex()
{
    static QMutex m;
    return &m;
}

static void rmExpiredLogs()
{
    QtConcurrent::run([=]() {
        QDirIterator itera(LogUtils::cachePath()
                           + "/" + QString(tcDirName));
        while (itera.hasNext()) {
            itera.next();
            auto list = itera.fileName().split("_");
            if (itera.fileInfo().suffix() == "csv"
                && list.count() == 3
                && !LogUtils::containLastDay(
                           QDateTime(QDate::fromString(list[0], "yyyy-MM-dd"),
                                     QTime(0, 0, 0, 0)),
                           LogUtils::toDayZero(),
                           GlobalPrivate::kDayCount)) {
                qInfo("remove true(%d) not last week log: %s",
                      QDir().remove(itera.path() + "/" + itera.fileName()),
                      itera.fileName().toLocal8Bit().data());
            }
        }
    });
}

static void outCheck(const QMessageLogContext &context, const QString &msg)
{
    // root user
    if (static_cast<int>(getuid()) == 0)
        return;

    //加锁保证内部函数执行时的互斥
    QMutexLocker lock(GlobalPrivate::mutex());

    const QString &currAppLogName = LogUtils::cachePath()
            + "/" + QString(tcDirName) + "/"
            + LogUtils::localDate() + "_" + QString(tcFileName);

    // "codeTimeCheck" dir
    if (!LogUtils::checkAppCacheLogDir(tcDirName))
        return;

    // 文件名称为空 或者当前日期不正确(跨天日志分割)
    if (file()->fileName().isEmpty()
        || file()->fileName() != currAppLogName) {
        file()->setFileName(currAppLogName);
        qInfo() << "Current checkTime file path: " << file()->fileName();
        rmExpiredLogs();
    }

    bool isNewFile = false;
    if (!file()->exists()) {
        isNewFile = true;
    }

    if (!file()->isOpen()) {
        file()->open(QFile::WriteOnly | QFile::Append);
    }

    if (isNewFile) {
        file()->write((QString("时间") + ","
                       + "时间" + ","
                       + "毫秒" + ","
                       + "函数名称" + ","
                       + "动作类型" + ","
                       + "文件名称" + ","
                       + "文件行" + "\n")
                              .toUtf8()
                              .data());
        file()->flush();
    }

    auto fileNameList = QString(context.file).split("/");
    auto currentName = fileNameList[fileNameList.size() - 1];

    file()->write((LogUtils::localDataTimeCSV() + ","
                   + context.function + ","
                   + msg + ","
                   + currentName + ","
                   + QString::number(context.line)
                   + "\n")
                          .toUtf8()
                          .data());
    file()->flush();
    file()->close();
}

}   // namespace GlobalPrivate

/*!
 * \class CodeCheckTime
 * \brief The CodeCheckTime class
 * 代码埋点时间检查模块，可加编译参数进行屏蔽
 * DPF_NO_CHECK_TIME (cmake -DDPF_NO_CHECK_TIME)
 */

/*!
 * \brief setLogCacheDayCount 设置日志缓存时间
 *  需要在调用其他函数之前调用
 * \param dayCount 日志缓存时间
 */
void CodeCheckTime::setLogCacheDayCount(uint dayCount)
{
    GlobalPrivate::kDayCount = dayCount;
}

/*!
 * \brief logCacheDayCount 获取设置的日志缓存时间
 * \return uint 日志缓存时间，默认7天
 */
uint CodeCheckTime::logCacheDayCount()
{
    return GlobalPrivate::kDayCount;
}

/*!
 * \brief begin 检查点-开始
 * \param context 日志打印上下文，可参照QMessageLogContext
 */
void CodeCheckTime::begin(const QMessageLogContext &context)
{
    GlobalPrivate::outCheck(context, "begin");
}

/*!
 * \brief end 检查点-结束
 * \param context 日志打印上下文，可参照QMessageLogContext
 */
void CodeCheckTime::end(const QMessageLogContext &context)
{
    GlobalPrivate::outCheck(context, "end");
}

#endif   // DPF_NO_CHECK_TIME

DPF_END_NAMESPACE
