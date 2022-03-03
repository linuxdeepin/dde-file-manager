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
#include <QFuture>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

class MainController : public QObject
{
    Q_OBJECT
    friend class SearchService;
    friend class SearchServicePrivate;

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

DSB_FM_END_NAMESPACE

#endif   // MAINCONTROLLER_H
