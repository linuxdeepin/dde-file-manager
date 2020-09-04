/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef BASEJOB_H
#define BASEJOB_H

#include <QObject>
#include <QDBusContext>

class BaseJob : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit BaseJob(QObject *parent = nullptr);
    ~BaseJob();


    qint64 getClientPid();
    bool checkAuthorization(const QString& actionId, qint64 applicationPid);

    QString objectPath() const;
    void setObjectPath(const QString &objectPath);

    bool registerObject();

signals:

public slots:

private:
    QString m_objectPath;
};

#endif // BASEJOB_H
