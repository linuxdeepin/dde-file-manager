// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "task/taskcommander.h"

#include <QHash>
#include <QFuture>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE
class MainController : public QObject
{
    Q_OBJECT
    friend class SearchManager;

private:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();
    void init();

    void stop(QString taskId);
    bool doSearchTask(QString taskId, const QUrl &url, const QString &keyword);
    QList<QUrl> getResults(QString taskId);

private slots:
    void onFinished(QString taskId);
    void onFileChanged(const QString &path);

signals:
    void matched(QString taskId);
    void searchCompleted(QString taskId);

private:
    QHash<QString, TaskCommander *> taskManager;
    QFileSystemWatcher *fileWatcher = nullptr;
    QFuture<void> indexFuture;
};

DPSEARCH_END_NAMESPACE

#endif   // MAINCONTROLLER_H
