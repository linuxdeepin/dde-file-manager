// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGGERRULES_H
#define LOGGERRULES_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

//!
//! \brief Refrence: https://gitlabwh.uniontech.com/
//! ut000683/deepin-docs/-/blob/main/debug.md
//!
//! The impl copy from: https://github.com/linuxdeepin/deepin-log-viewer
//! /pull/253/files
//!
//! verify: sudo deepin-debug-config -s -m all -l on
//!

class LoggerRules : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(LoggerRules)

public:
    static LoggerRules &instance();

    void initLoggerRules();
    QString rules() const;
    void setRules(const QString &rules);

private:
    explicit LoggerRules(QObject *parent = nullptr);
    ~LoggerRules();

    void appendRules(const QString &rules);

private:
    QString currentRules;
};

DFMBASE_END_NAMESPACE

#endif   // LOGGERRULES_H
