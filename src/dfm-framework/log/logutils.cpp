#include "logutils.h"

#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate
{
    const QString cachePath = QStandardPaths::locate(QStandardPaths::CacheLocation,
                                                     "",
                                                     QStandardPaths::LocateDirectory);
    const QString deepinCachePath = cachePath + "deepin/";
}

void LogUtils::checkAppCacheLogDir(const QString &subDirName)
{
    if (!QFileInfo::exists(GlobalPrivate::deepinCachePath))
        QDir().mkdir(appCacheLogPath());

    if (!QFileInfo::exists(appCacheLogPath()))
        QDir().mkdir(appCacheLogPath());

    if (subDirName.isEmpty()) return;

    if (!QFileInfo::exists(appCacheLogPath() + "/" + subDirName))
        QDir().mkdir(appCacheLogPath() + "/" + subDirName);
}

QString LogUtils::appCacheLogPath()
{
    return GlobalPrivate::deepinCachePath + QCoreApplication::applicationName();
}

QString LogUtils::localDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString LogUtils::localDate()
{
    return QDate::currentDate().toString("yyyy-MM-dd");
}

QString LogUtils::localDataTimeCSV()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd,hh:mm:ss,zzz");
}

uint LogUtils::lastTimeStamp(const QDateTime &dateTime, uint dayCount)
{
    return dateTime.toTime_t() - (86400 * dayCount);
}

QDateTime LogUtils::lastDateTime(const QDateTime &dateTime, uint dayCount)
{
    return QDateTime::fromTime_t(lastTimeStamp(dateTime,dayCount));
}

bool LogUtils::containLastDay(const QDateTime &src, const QDateTime &dst, uint dayCount)
{
    uint srcStamp = src.toTime_t();
    uint dstStamp = dst.toTime_t();

    return dstStamp - (86400 * dayCount) < srcStamp && srcStamp <= dstStamp;
}

QDateTime LogUtils::toDayZero()
{
    QDateTime dateTime;
    dateTime.setDate(QDate::currentDate());
    dateTime.setTime(QTime(0,0,0));
    return dateTime;
}

DPF_END_NAMESPACE
