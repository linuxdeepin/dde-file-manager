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
#include "fulltext/fulltextsearcher.h"

#include <QFileSystemWatcher>
#include <QApplication>
#include <QtConcurrent>
#include <QUrl>
#include <QDir>
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

void MainController::stop(QString taskId)
{
    if (taskManager.contains(taskId)) {
        disconnect(taskManager[taskId]);
        taskManager[taskId]->stop();
        taskManager[taskId]->deleteSelf();
        taskManager[taskId] = nullptr;
        taskManager.remove(taskId);
    }
}

bool MainController::doSearchTask(QString taskId, const DUrl &url, const QString &keyword)
{
    if (taskManager.contains(taskId))
        stop(taskId);

    auto task = new TaskCommander(taskId, url, keyword);
    qInfo() << "new task: " << task << task->taskID();
    Q_ASSERT(task);
    taskManager.insert(taskId, task);

    //直连，防止1被事件循环打乱时序
    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::DirectConnection);
    connect(task, &TaskCommander::finished, this, &MainController::onFinished, Qt::DirectConnection);

    if (task->start())
        return true;

    qWarning() << "fail to start task " << task << task->taskID();
    task->deleteSelf();
    return false;
}

QList<DUrl> MainController::getResults(QString taskId)
{
    if (taskManager.contains(taskId))
        return taskManager[taskId]->getResults();

    return {};
}

void MainController::createFullTextIndex()
{
    if (!indexFuture.isRunning()) {
        indexFuture = QtConcurrent::run([]() {
            FullTextSearcher searcher(DUrl(), "");
            searcher.createIndex("/");
        });
    }
}

void MainController::onFinished(QString taskId)
{
    if (taskManager.contains(taskId))
        stop(taskId);

    emit searchCompleted(taskId);
}
