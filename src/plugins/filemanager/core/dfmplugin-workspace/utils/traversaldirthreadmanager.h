// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSALDIRTHREADMANAGER_H
#define TRAVERSALDIRTHREADMANAGER_H

#include "dfmplugin_workspace_global.h"
#include "dfm_global_defines.h"
#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QThread>
#include <QUrl>
#include <QElapsedTimer>

#include <dfm-io/core/denumerator.h>

using namespace dfmbase;

namespace dfmplugin_workspace {

class TraversalDirThreadManager : public TraversalDirThread
{
    Q_OBJECT
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
    dfmio::DEnumerator::SortRoleCompareFlag sortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    bool isMixDirAndFile { false };

public:
    explicit TraversalDirThreadManager(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                       QDir::Filters filters = QDir::NoFilter,
                                       QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                       QObject *parent = nullptr);
    virtual ~TraversalDirThreadManager() override;
    void setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole, const bool isMixDirAndFile);

Q_SIGNALS:
    void updateChildrenManager(QList<AbstractFileInfoPointer> children);
    void updateChildManager(const AbstractFileInfoPointer child);
    // Special processing If it is a local file, directly read all the simple sorting lists of the file
    void updateLocalChildren(QList<SortInfoPointer> children,
                             dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                             Qt::SortOrder sortOrder,
                             bool isMixDirAndFile);
    void traversalFinished();

protected:
    virtual void run() override;

private:
    int iteratorOneByOne(const QElapsedTimer &timer);
    int iteratorAll();
};
}

typedef QSharedPointer<dfmplugin_workspace::TraversalDirThreadManager> TraversalThreadManagerPointer;
#endif   // TRAVERSALDIRTHREADMANAGER_H
