// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSEARCHER_H
#define ABSTRACTSEARCHER_H

#include "dfmplugin_search_global.h"

#include <QObject>
#include <QUrl>

DPSEARCH_BEGIN_NAMESPACE

struct ContentSearchResult {
    QUrl url {};
    QString highlightedContent {};
};

class AbstractSearcher : public QObject
{
    Q_OBJECT
public:
    enum Status {
        kReady,
        kRuning,
        kCompleted,
        kTerminated
    };

    explicit AbstractSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);
    virtual bool search() = 0;
    // virtual void stop() = 0;
    virtual bool hasItem() const = 0;
    virtual QList<QUrl> takeAll() = 0;
    virtual QMap<QUrl, ContentSearchResult> takeContentResults() = 0;
signals:
    void unearthed(AbstractSearcher *searcher);
    void finished();

protected:
    QUrl searchUrl;
    QString keyword;
};

DPSEARCH_END_NAMESPACE

#endif   // ABSTRACTSEARCHER_H
