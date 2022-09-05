// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
