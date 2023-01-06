// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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

class MainController : public QObject
{
    Q_OBJECT
    friend class SearchService;
    friend class SearchServicePrivate;

private:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    void stop(QString taskId);
    bool doSearchTask(QString taskId, const DUrl &url, const QString &keyword);
    QList<DUrl> getResults(QString taskId);
    void createFullTextIndex();

private slots:
    void onFinished(QString taskId);

signals:
    void matched(QString taskId);
    void searchCompleted(QString taskId);

private:
    QHash<QString, TaskCommander *> taskManager;
    QFuture<void> indexFuture;
};

#endif   // MAINCONTROLLER_H
