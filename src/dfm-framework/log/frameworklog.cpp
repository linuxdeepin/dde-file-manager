#include "frameworklog.h"

#include <QDateTime>

Q_LOGGING_CATEGORY(FrameworkLog, "FrameworkLog")
Q_LOGGING_CATEGORY(TimeCheck,"TimeCheck")

DPF_BEGIN_NAMESPACE

void enableFrameworkLog(bool enabled){
    if (enabled) {
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.warning=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.debug=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.info=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.critical=true"));

    } else {
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.warning=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.debug=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.info=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.critical=false"));
    }
}


void enableTimeCheck(bool enabled)
{
    if (enabled) {
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.warning=true"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.debug=true"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.info=true"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.critical=true"));

    } else {
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.warning=false"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.debug=false"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.info=false"));
        QLoggingCategory::setFilterRules(QLatin1String("TimeCheck.critical=false"));
    }
}

QString localDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
}

DPF_END_NAMESPACE
