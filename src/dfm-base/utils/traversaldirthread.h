// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSALDIRTHREAD_H
#define TRAVERSALDIRTHREAD_H

#include "dfm_base_global.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QMetaType>
#include <QObject>
#include <QThread>
#include <QUrl>

namespace dfmbase {

class TraversalDirThread : public QThread
{
    Q_OBJECT
    QUrl dirUrl;   // 遍历的目录的url
    QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> dirIterator;   // 当前遍历目录的diriterator
    QList<QUrl> childrenList;   // 当前遍历出来的所有文件
    bool stopFlag = false;

    QStringList nameFilters;
    QDir::Filters filters;
    QDirIterator::IteratorFlags flags;

public:
    explicit TraversalDirThread(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                QDir::Filters filters = QDir::NoFilter,
                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;
    void stop();
    void quit();
    void stopAndDeleteLater();

Q_SIGNALS:
    void updateChildren(const QList<QUrl> children);
    void updateChild(const QUrl url);
    void stoped();

protected:
    virtual void run() override;
};
}

typedef QSharedPointer<DFMBASE_NAMESPACE::TraversalDirThread> TraversalThreadPointer;

#endif   // DFMTRAVERSALDIRTHREAD_H
