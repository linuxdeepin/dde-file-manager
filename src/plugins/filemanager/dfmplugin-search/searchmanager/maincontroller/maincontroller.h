// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "task/taskcommander.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <QHash>
#include <QFuture>

DPSEARCH_BEGIN_NAMESPACE
class MainController : public QObject
{
    Q_OBJECT
    friend class SearchManager;

private:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    bool doSearchTask(QString taskId, const QUrl &url, const QString &keyword);
    void stop(QString taskId);
    
    // 获取统一的搜索结果
    DFMSearchResultMap getResults(QString taskId);
    
    // 为兼容性保留的接口
    QList<QUrl> getResultUrls(QString taskId);

private slots:
    void onFinished(QString taskId);

signals:
    void matched(QString taskId);
    void searchCompleted(QString taskId);

private:
    QHash<QString, TaskCommander *> taskManager;
    QFuture<void> indexFuture;
};

DPSEARCH_END_NAMESPACE

#endif   // MAINCONTROLLER_H
