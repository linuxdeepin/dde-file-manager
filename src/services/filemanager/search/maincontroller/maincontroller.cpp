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
#include "maincontroller.h"

#include <QDebug>

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

MainController::~MainController()
{
    for (auto &task : taskManager) {
        task->stop();
        task->deleteSelf();
        task = nullptr;
    }
    taskManager.clear();
}

void MainController::stop(quint64 winId)
{
    if (taskManager.contains(winId)) {
        disconnect(taskManager[winId]);
        taskManager[winId]->stop();
        taskManager[winId]->deleteSelf();
        taskManager[winId] = nullptr;
        taskManager.remove(winId);
    }
}

bool MainController::doSearchTask(quint64 winId, const QUrl &url, const QString &keyword)
{
    if (taskManager.contains(winId))
        stop(winId);

    auto task = new TaskCommander(winId, url, keyword);
    qInfo() << "new task: " << task << task->taskID();
    Q_ASSERT(task);
    taskManager.insert(winId, task);

    //直连，防止被事件循环打乱时序
    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::DirectConnection);
    connect(task, &TaskCommander::finished, this, &MainController::searchCompleted, Qt::DirectConnection);

    if (task->start())
        return true;

    qWarning() << "fail to start task " << task << task->taskID();
    task->deleteSelf();
    return false;
}

QStringList MainController::getResults(quint64 winId)
{
    if (taskManager.contains(winId))
        return taskManager[winId]->getResults();

    return {};
}
