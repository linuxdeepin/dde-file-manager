// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHINGMONITORFILTER_H
#define ANYTHINGMONITORFILTER_H

#include "dfmplugin_tag_global.h"

#include <QObject>
#include <QMap>

DPTAG_BEGIN_NAMESPACE

class AnythingMonitorFilter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AnythingMonitorFilter)

public:
    static AnythingMonitorFilter &instance();
    bool whetherFilterCurrentPath(const QString &localPath);

private:
    explicit AnythingMonitorFilter(QObject *parent = nullptr);
    virtual ~AnythingMonitorFilter();

    void readSettings();
    void readHomePathOfAllUsers();
    QString restoreEscapedChar(const QString &value);
    void reserveDir(QStringList *list);

private:
    QStringList blackList;
    QStringList whiteList;
    std::map<QString, QString> userNameAndHomePath;
};

DPTAG_END_NAMESPACE

#endif   // ANYTHINGMONITORFILTER_H
