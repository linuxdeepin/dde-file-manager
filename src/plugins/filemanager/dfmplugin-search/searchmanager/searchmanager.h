// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "dfmplugin_search_global.h"

#include <QObject>
#include <QMap>

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
    QList<QUrl> matchedResults(const QString &taskId);
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
    QMap<quint64, QString> taskIdMap;
};

}

#endif   // SEARCHMANAGER_H
