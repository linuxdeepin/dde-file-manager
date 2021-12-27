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

class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT
    friend class MainController;

private:
    explicit TaskCommander(quint64 taskId, const QUrl &url, const QString &keyword, QObject *parent = nullptr);
    quint64 taskID() const;
    QStringList getResults() const;
    bool start();
    void stop();
    void deleteSelf();
    void createSearcher(const QUrl &url, const QString &keyword);

signals:
    void matched(quint64 taskId);
    void finished(quint64 taskId);

private:
    TaskCommanderPrivate *d;
};

#endif   // TASKCOMMANDER_H
