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
#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "task/taskcommander.h"

#include <QHash>

class MainControllerPrivate;
class MainController : public QObject
{
    Q_OBJECT
    friend class SearchService;
    friend class SearchServicePrivate;

private:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();
    bool init();

    void stop(quint64 winId);
    bool doSearchTask(quint64 winId, const QUrl &url, const QString &keyword);
    QStringList getResults(quint64 winId);

signals:
    void matched(quint64 winId);
    void searchCompleted(quint64 winId);

private:
    QHash<quint64, TaskCommander *> taskManager;
};

#endif   // MAINCONTROLLER_H
