// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "dfmplugin_search_global.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <QObject>
#include <QMap>
#include <QMultiMap>

namespace dfmplugin_search {

class MainController;
class SearchManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchManager)

public:
    static SearchManager *instance();

    void init();
    bool search(quint64 winId, const QString &taskId, const QUrl &url, const QString &keyword);
    
    // 获取统一的搜索结果数据
    DFMSearchResultMap matchedResults(const QString &taskId);
    
    // 为向后兼容保留的接口，只获取URL列表
    QList<QUrl> matchedResultUrls(const QString &taskId);
    
    void stop(const QString &taskId);
    void stop(quint64 winId);

public Q_SLOTS:
    void onDConfigValueChanged(const QString &config, const QString &key);

signals:
    void matched(const QString &taskId);
    void searchCompleted(const QString &taskId);
    void searchStoped(const QString &taskId);
    void enableFullTextSearchChanged(bool enable);

    void fileAdd(const QUrl &url);
    void fileDelete(const QUrl &url);
    void fileRename(const QUrl &oldUrl, const QUrl &newUrl);

private:
    explicit SearchManager(QObject *parent = nullptr);
    ~SearchManager();

    MainController *mainController = nullptr;
    QMap<quint64, QString> taskIdMap;  // 当前窗口最近一次的搜索taskId
    QMultiMap<quint64, QString> winTasksMap;  // 窗口ID对应的所有搜索任务ID
    QMap<QString, QPair<QUrl, QString>> taskInfoMap; // 保存任务ID对应的url和keyword
};

}

#endif   // SEARCHMANAGER_H
