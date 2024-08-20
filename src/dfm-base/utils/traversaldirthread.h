// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSALDIRTHREAD_H
#define TRAVERSALDIRTHREAD_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <QThread>
#include <QUrl>

namespace dfmbase {

class TraversalDirThread : public QThread
{
    Q_OBJECT
protected:
    QUrl dirUrl;   // 遍历的目录的url
    QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> dirIterator;   // 当前遍历目录的diriterator
    QStringList nameFilters;
    QDir::Filters filters;
    QDirIterator::IteratorFlags flags;
    QList<QUrl> childrenList;   // 当前遍历出来的所有文件
    bool stopFlag = false;
    QString fileInfoQueryAttributes;

public:
    explicit TraversalDirThread(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                QDir::Filters filters = QDir::NoFilter,
                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;
    void stop();
    void quit();
    void stopAndDeleteLater();
    void setQueryAttributes(const QString &fileAttributes);

Q_SIGNALS:
    void updateChildren(QList<QUrl> children);
    void updateChild(const QUrl child);
    void stoped();

protected:
    virtual void run() override;
};
}

typedef QSharedPointer<DFMBASE_NAMESPACE::TraversalDirThread> TraversalThreadPointer;
#endif   // DFMTRAVERSALDIRTHREAD_H
