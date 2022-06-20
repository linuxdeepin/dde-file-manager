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
#include "search/searcher/fulltextsearcher/fulltextsearcher.h"

#include "dfm-base/base/application/settings.h"

#include <QFileSystemWatcher>
#include <QApplication>
#include <QtConcurrent>
#include <QUrl>
#include <QDir>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

MainController::MainController(QObject *parent)
    : QObject(parent)
{
    init();
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

void MainController::init()
{
    fileWatcher = new QFileSystemWatcher(this);
    auto configPath = QDir::home().absoluteFilePath(".config/deepin/dde-file-manager.json");
    fileWatcher->addPath(configPath);
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainController::onFileChanged);
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

bool MainController::doSearchTask(QString taskId, const QUrl &url, const QString &keyword)
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

QList<QUrl> MainController::getResults(QString taskId)
{
    if (taskManager.contains(taskId))
        return taskManager[taskId]->getResults();

    return {};
}

void MainController::onFinished(QString taskId)
{
    if (taskManager.contains(taskId))
        stop(taskId);

    emit searchCompleted(taskId);
}

void MainController::onFileChanged(const QString &path)
{
    Q_UNUSED(path);

    Settings settings("deepin/dde-file-manager", Settings::kGenericConfig);
    bool value = settings.value("GenericAttribute", "IndexFullTextSearch", false).toBool();
    if (value && !indexFuture.isRunning()) {
        indexFuture = QtConcurrent::run([]() {
            FullTextSearcher searcher(QUrl(), "");
            searcher.createIndex("/");
        });
    }
}

DSB_FM_END_NAMESPACE
