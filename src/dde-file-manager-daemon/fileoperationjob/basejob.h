// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
