// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_H
#define FULLTEXTSEARCHER_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <QObject>

#include <memory>

class OrgDeepinFilemanagerTextIndexInterface;

DPSEARCH_BEGIN_NAMESPACE

class FullTextSearcherPrivate;
class FullTextSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class MainController;
    friend class FullTextSearcherPrivate;

private:
    explicit FullTextSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);
    ~FullTextSearcher();

    bool createIndex(const QString &path);
    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<QUrl> takeAll() override;
    static bool isSupport(const QUrl &url);

private:
    std::unique_ptr<OrgDeepinFilemanagerTextIndexInterface> interface;
    FullTextSearcherPrivate *d = nullptr;
};

DPSEARCH_END_NAMESPACE

#endif   // FULLTEXTSEARCHER_H
