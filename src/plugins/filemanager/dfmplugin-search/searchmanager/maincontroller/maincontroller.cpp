// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maincontroller.h"

#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/application/application.h>

#include <QApplication>
#include <QtConcurrent>
#include <QUrl>
#include <QDir>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

MainController::~MainController()
{
    for (auto &task : taskManager) {
        task->deleteSelf();
        task = nullptr;
    }
    taskManager.clear();
}

bool MainController::doSearchTask(QString taskId, const QUrl &url, const QString &keyword)
{
    if (taskManager.contains(taskId))
        taskManager[taskId]->deleteSelf();

    auto task = new TaskCommander(taskId, url, keyword);
    Q_ASSERT(task);
    fmInfo() << "new task: " << task << task->taskID();

    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::QueuedConnection);
    connect(task, &TaskCommander::finished, this, &MainController::onFinished, Qt::QueuedConnection);

    if (task->start()) {
        taskManager.insert(taskId, task);
        return true;
    }

    fmWarning() << "fail to start task " << task << task->taskID();
    task->deleteSelf();
    return false;
}

void MainController::stop(QString taskId)
{
    if (taskManager.contains(taskId))
        taskManager[taskId]->stop();
}

DFMSearchResultMap MainController::getResults(QString taskId)
{
    if (taskManager.contains(taskId))
        return taskManager[taskId]->getResults();

    return {};
}

QList<QUrl> MainController::getResultUrls(QString taskId)
{
    if (taskManager.contains(taskId))
        return taskManager[taskId]->getResultsUrls();

    return {};
}

void MainController::onFinished(QString taskId)
{
    emit searchCompleted(taskId);

    if (taskManager.contains(taskId)) {
        taskManager[taskId]->deleteSelf();
        taskManager.remove(taskId);
    }
}
