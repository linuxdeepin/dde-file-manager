// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_H
#define FULLTEXTSEARCHER_H

#include "abstractsearcher.h"

#include <QObject>

class FullTextSearcherPrivate;
class FullTextSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class MainController;
    friend class FullTextSearcherPrivate;

private:
    explicit FullTextSearcher(const DUrl &url, const QString &key, QObject *parent = nullptr);
    bool createIndex(const QString &path);
    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<DUrl> takeAll() override;
    static bool isSupport(const DUrl &url);

private:
    FullTextSearcherPrivate *d = nullptr;
};

#endif   // FULLTEXTSEARCHER_H
