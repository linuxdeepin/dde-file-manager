// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_H
#define TASKCOMMANDER_H

#include "dfmplugin_search_global.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <QObject>
#include <QUrl>

DPSEARCH_BEGIN_NAMESPACE

class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT
    friend class MainController;

private:
    explicit TaskCommander(QString taskId, const QUrl &url, const QString &keyword, QObject *parent = nullptr);
    ~TaskCommander();
    
    // 任务标识
    QString taskID() const;
    
    // 获取搜索结果
    DFMSearchResultMap getResults() const;
    QList<QUrl> getResultsUrls() const;
    
    // 控制搜索流程
    bool start();
    void stop();
    // void deleteSelf();
    
    // 准备销毁任务，非阻塞方式停止线程
    // void prepareForDestroy();

public Q_SLOTS:
    void onWorkThreadFinished();

signals:
    void matched(QString taskId);
    void finished(QString taskId);
    void threadFinished(QString taskId);

private:
    TaskCommanderPrivate *d;
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_H
