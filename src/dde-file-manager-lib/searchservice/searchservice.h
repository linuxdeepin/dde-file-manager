// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include "durl.h"

#include <QObject>

#define searchServ SearchService::instance()

class SearchServicePrivate;
class SearchService : public QObject
{
    Q_OBJECT
public:
    static SearchService *instance();

    bool search(const QString &taskId, const DUrl &url, const QString &keyword);
    QList<DUrl> matchedResults(const QString &taskId);
    void stop(const QString &taskId);
    void createFullTextIndex();

signals:
    void matched(const QString &taskId);
    void searchCompleted(const QString &taskId);
    void searchStoped(const QString &taskId);

private:
    explicit SearchService(QObject *parent = nullptr);
    ~SearchService();
    void init();

    QScopedPointer<SearchServicePrivate> d;
};

#endif   // SEARCHSERVICE_H
