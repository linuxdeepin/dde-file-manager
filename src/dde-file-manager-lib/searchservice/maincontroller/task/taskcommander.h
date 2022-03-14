/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
