/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
