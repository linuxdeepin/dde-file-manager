// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAVERSALDIRTHREADMANAGER_H
#define TRAVERSALDIRTHREADMANAGER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/traversaldirthread.h>

#include <QThread>
#include <QUrl>
#include <QElapsedTimer>

#include <dfm-io/denumerator.h>
#include <dfm-io/denumeratorfuture.h>

using namespace dfmbase;

namespace dfmplugin_workspace {

class TraversalDirThreadManager : public TraversalDirThread
{
    Q_OBJECT
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
    dfmio::DEnumerator::SortRoleCompareFlag sortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    bool isMixDirAndFile { false };
    QElapsedTimer *timer = Q_NULLPTR;
    int timeCeiling = 1500;
    int countCeiling = 500;
    dfmio::DEnumeratorFuture *future { nullptr };
    QString traversalToken;
    std::atomic_bool running = false;

public:
    explicit TraversalDirThreadManager(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                       QDir::Filters filters = QDir::NoFilter,
                                       QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                       QObject *parent = nullptr);
    virtual ~TraversalDirThreadManager() override;
    void setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole, const bool isMixDirAndFile);
    void setTraversalToken(const QString &token);

    void start();

    bool isRunning() const;

public Q_SLOTS:
    void onAsyncIteratorOver();

Q_SIGNALS:
    void updateChildrenManager(const QList<FileInfoPointer> children, QString traversalToken);
    // Special processing If it is a local file, directly read all the simple sorting lists of the file
    void updateLocalChildren(const QList<SortInfoPointer> children,
                             dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                             Qt::SortOrder sortOrder,
                             bool isMixDirAndFile, QString traversalToken);
    void traversalFinished(QString traversalToken);
    void traversalRequestSort(QString traversalToken);

    void iteratorInitFinished();

protected:
    virtual void run() override;

private:
    int iteratorOneByOne(const QElapsedTimer &timere);
    QList<SortInfoPointer> iteratorAll();
    void createFileInfo(const QList<SortInfoPointer> &list);
};
}

typedef QSharedPointer<dfmplugin_workspace::TraversalDirThreadManager> TraversalThreadManagerPointer;
#endif   // TRAVERSALDIRTHREADMANAGER_H
