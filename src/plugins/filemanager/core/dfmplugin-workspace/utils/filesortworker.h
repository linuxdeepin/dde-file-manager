// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESORTWORKER_H
#define FILESORTWORKER_H

#include "dfmplugin_workspace_global.h"
#include "dfm_global_defines.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/interfaces/abstractsortandfiter.h"

#include <dfm-io/core/denumerator.h>

#include <QObject>
#include <QDirIterator>
#include <QReadWriteLock>

using namespace dfmbase;
namespace dfmplugin_workspace {
class FileSortWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileSortWorker(const QUrl &url,
                            const QStringList &nameFilters = QStringList(),
                            const QDir::Filters filters = QDir::NoFilter,
                            const QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                            QObject *parent = nullptr);
    void setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole);
    int mapToIndex(int index);
    int childrenCount();
    void cancel();
signals:
    // All data is ready
    void allDataReady();
    // All data sorting completed
    void allDataSortedCompleted();
    // one file data add completed
    void addFileCompleted(const int index);
    // one file data add completed
    void removeFileCompleted(const int index);

    // Note that the slot functions here are executed in asynchronous threads,
    // so the link can only be Qt:: QueuedConnection,
    // which cannot be directly called elsewhere, but can only be triggered by signals
public slots:
    // Receive all local files of iteration of iterator thread, perform filtering and sorting
    void updateSortChildren(QList<QSharedPointer<DFMIO::DEnumerator::SortFileInfo>> children,
                            dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            Qt::SortOrder sortOrder,
                            bool isMixDirAndFile);
    void updateChild(const AbstractFileInfoPointer child);
    //Get data from the data area according to the url, filter and sort the data
    void getDataAndUpdate();
    void resetFilters(const QStringList &nameFilters = QStringList(),
                      const QDir::Filters filters = QDir::NoFilter);
    void addChild(const QSharedPointer<DFMIO::DEnumerator::SortFileInfo> &sortInfo);
    void removeChild(const QSharedPointer<DFMIO::DEnumerator::SortFileInfo> &sortInfo);
    void resort(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole);
private:
    bool checkFilters(const QSharedPointer<DFMIO::DEnumerator::SortFileInfo> &sortInfo);
    void filterAllFiles();
    void sortAllFiles();
    void updateChild(const QSharedPointer<DFMIO::DEnumerator::SortFileInfo> &sortInfo,
                     const AbstractSortAndFiter::SortScenarios sort);

private:
    bool lessThan(const int left,const int right, AbstractSortAndFiter::SortScenarios sort);
    QVariant data(const AbstractFileInfoPointer &info, dfmbase::Global::ItemRoles role);
    int insertSortList(const int needNode, const QList<int> &list, AbstractSortAndFiter::SortScenarios sort);

private:
    QUrl current;
    QStringList nameFilters;
    QDir::Filters filters{ QDir::NoFilter };
    QDirIterator::IteratorFlags flags{ QDirIterator::NoIteratorFlags };
    QList<QSharedPointer<DFMIO::DEnumerator::SortFileInfo>> children;
    QList<int> visibleChildrenIndex;
    QReadWriteLock locker;
    AbstractSortAndFiterPointer sortAndFilter { nullptr };
    dfmbase::Global::ItemRoles orgSortRole{ dfmbase::Global::ItemRoles::kItemFileDisplayNameRole };
    Qt::SortOrder sortOrder{ Qt::AscendingOrder };
    dfmio::DEnumerator::SortRoleCompareFlag sortRole{ dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    bool isMixDirAndFile { false };
    std::atomic_bool isCanceled { false };
    char placeholderMemory[5];
};

}

#endif // FILESORTWORKER_H
