// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loggerrules.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_BEGIN_NAMESPACE

LoggerRules &LoggerRules::instance()
{
    static LoggerRules rules;
    return rules;
}

void LoggerRules::initLoggerRules()
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    qunsetenv("QT_LOGGING_RULES");

    // set env
    currentRules = QString(logRules);
    qCWarning(logDFMBase) << "Current system env log rules:" << logRules;

    logRules = DConfigManager::instance()->value(kDefaultCfgPath, "log_rules").toByteArray();
    qCWarning(logDFMBase) << "Current app log rules :" << logRules;
    appendRules(logRules);
    setRules(currentRules);

    // watch dconfig
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, [this](const QString &config, const QString &key) {
        if (config == kDefaultCfgPath && key == "log_rules") {
            setRules(DConfigManager::instance()->value(kDefaultCfgPath, key).toByteArray());
            qCWarning(logDFMBase) << "value changed:" << key;
        }
    });
}

QString LoggerRules::rules() const
{
    return currentRules;
}

void LoggerRules::setRules(const QString &rules)
{
    auto tmpRules = rules;
    currentRules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(currentRules);
}

LoggerRules::LoggerRules(QObject *parent)
    : QObject(parent)
{
}

LoggerRules::~LoggerRules()
{
}

void LoggerRules::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++)
        if (currentRules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            i--;
        }
    if (tmplist.isEmpty())
        return;
    currentRules.isEmpty() ? currentRules = tmplist.join("\n")
                           : currentRules += "\n" + tmplist.join("\n");
}

DFMBASE_END_NAMESPACE
