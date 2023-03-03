// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSALDIRTHREAD_H
#define TRAVERSALDIRTHREAD_H

#include "dfm_base_global.h"
#include "dfm_global_defines.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QMetaType>
#include <QObject>
#include <QThread>
#include <QUrl>

#include <dfm-io/core/denumerator.h>

namespace dfmbase {

class TraversalDirThread : public QThread
{
    Q_OBJECT
    QUrl dirUrl;   // 遍历的目录的url
    QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> dirIterator;   // 当前遍历目录的diriterator
    QList<AbstractFileInfoPointer> childrenList;   // 当前遍历出来的所有文件
    bool stopFlag = false;
    bool isMixDirAndFile = false;

    QStringList nameFilters;
    QDir::Filters filters;
    QDirIterator::IteratorFlags flags;
    Qt::SortOrder sortOrder{ Qt::AscendingOrder};
    dfmio::DEnumerator::SortRoleCompareFlag sortRole{ dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };

public:
    explicit TraversalDirThread(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                QDir::Filters filters = QDir::NoFilter,
                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;
    void stop();
    void quit();
    void stopAndDeleteLater();
    void setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole, const bool isMixDirAndFile = false);

Q_SIGNALS:
    void updateChildren(QList<AbstractFileInfoPointer> children);
    void updateChild(const AbstractFileInfoPointer child);
    void stoped();
// Special processing If it is a local file, directly read all the simple sorting lists of the file
Q_SIGNALS:
    void updateLocalChildren(QList<QSharedPointer<DFMIO::DEnumerator::SortFileInfo>> children,
                             dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                             Qt::SortOrder sortOrder,
                             bool isMixDirAndFile);
protected:
    virtual void run() override;
};
}

typedef QSharedPointer<DFMBASE_NAMESPACE::TraversalDirThread> TraversalThreadPointer;
#endif   // DFMTRAVERSALDIRTHREAD_H
