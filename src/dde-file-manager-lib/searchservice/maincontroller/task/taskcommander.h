// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_H
#define TASKCOMMANDER_H

#include <QObject>

#include "durl.h"

class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT
    friend class MainController;

private:
    explicit TaskCommander(QString taskId, const DUrl &url, const QString &keyword, QObject *parent = nullptr);
    QString taskID() const;
    QList<DUrl> getResults() const;
    bool start();
    void stop();
    void deleteSelf();
    void createSearcher(const DUrl &url, const QString &keyword);

signals:
    void matched(QString taskId);
    void finished(QString taskId);

private:
    TaskCommanderPrivate *d;
};

#endif   // TASKCOMMANDER_H
