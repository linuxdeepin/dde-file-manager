// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSEARCHER_H
#define ABSTRACTSEARCHER_H

#include "durl.h"

#include <QObject>

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

    explicit AbstractSearcher(const DUrl &url, const QString &key, QObject *parent = nullptr);
    virtual bool search() = 0;
    virtual void stop() = 0;
    virtual bool hasItem() const = 0;
    virtual QList<DUrl> takeAll() = 0;
signals:
    void unearthed(AbstractSearcher *searcher);

protected:
    DUrl searchUrl;
    QString keyword;
};

#endif   // ABSTRACTSEARCHER_H
