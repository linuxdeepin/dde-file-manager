// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESORTWORKER_H
#define FILESORTWORKER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractsortfilter.h>
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <dfm-base/base/application/application.h>

#include <dfm-io/denumerator.h>

#include <QObject>
#include <QDirIterator>
#include <QReadWriteLock>

using namespace dfmbase;
namespace dfmplugin_workspace {
class FileItemData;
class FileSortWorker : public QObject
{
    Q_OBJECT
    enum class SortOpt : uint8_t {
        kSortOptNone = 0,
        kSortOptOnlyOrderChanged = 1,
        kSortOptOtherChanged = 2,
    };

public:
    explicit FileSortWorker(const QUrl &url,
                            const QString &key,
                            FileViewFilterCallback callfun = nullptr,
                            const QStringList &nameFilters = QStringList(),
                            const QDir::Filters filters = QDir::NoFilter,
                            const QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                            QObject *parent = nullptr);
    ~FileSortWorker();
    SortOpt setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole,
                             const bool isMixDirAndFile);
    QUrl mapToIndex(int index);
    int childrenCount();
    FileItemData *childData(const int index);
    FileItemData *childData(const QUrl &url);
    void setRootData(FileItemData *data);
    FileItemData *rootData() const;
    void cancel();
    int getChildShowIndex(const QUrl &url);
    QList<QUrl> getChildrenUrls();

    QDir::Filters getFilters() const;

    DFMGLOBAL_NAMESPACE::ItemRoles getSortRole() const;
    Qt::SortOrder getSortOrder() const;

signals:
    void insertRows(int first, int count);
    void insertFinish();
    void removeRows(int first, int count);
    void removeFinish();
    void requestFetchMore();
    void selectAndEditFile(const QUrl &url);

    void requestSetIdel();
    void updateRow(int row);

    // notify data
    void getSourceData(const QString &key);

    void requestUpdateView();

    // Note that the slot functions here are executed in asynchronous threads,
    // so the link can only be Qt:: QueuedConnection,
    // which cannot be directly called elsewhere, but can only be triggered by signals
public slots:
    // Receive all local files of iteration of iterator thread, perform filtering and sorting
    void handleIteratorLocalChildren(const QString &key,
                                     QList<SortInfoPointer> children,
                                     const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                                     const Qt::SortOrder sortOrder,
                                     const bool isMixDirAndFile);
    void handleSourceChildren(const QString &key,
                              QList<SortInfoPointer> children,
                              const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                              const Qt::SortOrder sortOrder,
                              const bool isMixDirAndFile,
                              const bool isFinished);
    void handleIteratorChild(const QString &key, const SortInfoPointer child, const FileInfoPointer info);
    void handleIteratorChildren(const QString &key, QList<SortInfoPointer> children, QList<FileInfoPointer> infos);
    // Get data from the data area according to the url, filter and sort the data
    void handleModelGetSourceData();
    void setFilters(QDir::Filters filters);
    void setNameFilters(const QStringList &filters);
    void resetFilters(const QDir::Filters filters = QDir::NoFilter);
    void onToggleHiddenFiles();
    void onShowHiddenFileChanged(bool isShow);
    void onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value);

    void handleWatcherAddChildren(QList<SortInfoPointer> children);
    void handleWatcherRemoveChildren(QList<SortInfoPointer> children);
    void resort(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile);
    void handleTraversalFinish(const QString &key);
    void handleSortAll(const QString &key);
    void handleWatcherUpdateFile(const SortInfoPointer child);
    void handleWatcherUpdateHideFile(const QUrl &hidUrl);
    void handleUpdateFile(const QUrl &url);
    void handleFilterData(const QVariant &data);
    void handleFilterCallFunc(FileViewFilterCallback callback);
    void handleRefresh();
    void handleClearThumbnail();
    void handleFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg);

private:
    void checkNameFilters(FileItemData *itemData);
    bool checkFilters(const SortInfoPointer &sortInfo, const bool byInfo = false);
    void filterAllFiles(const bool byInfo = false);
    void filterAllFilesOrdered();
    void sortAllFiles();
    // 有序的情况下只是点击升序还是降序特殊处理
    void sortOnlyOrderChange();
    void addChild(const SortInfoPointer &sortInfo, const FileInfoPointer &info);
    void addChild(const SortInfoPointer &sortInfo,
                  const AbstractSortFilter::SortScenarios sort);
    bool sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo);

private:
    bool lessThan(const QUrl &left, const QUrl &right, AbstractSortFilter::SortScenarios sort);
    QVariant data(const FileInfoPointer &info, Global::ItemRoles role);
    int insertSortList(const QUrl &needNode, const QList<QUrl> &list,
                       AbstractSortFilter::SortScenarios sort);
    bool isDefaultHiddenFile(const QUrl &fileUrl);

private:
    QUrl current;
    QStringList nameFilters {};
    QDir::Filters filters { QDir::NoFilter };
    QDirIterator::IteratorFlags flags { QDirIterator::NoIteratorFlags };
    QList<SortInfoPointer> children {};
    QList<QUrl> childrenUrlList {};
    QReadWriteLock childrenDataLocker;
    QMap<QUrl, FileItemData *> childrenDataMap {};
    QList<QUrl> visibleChildren {};
    QReadWriteLock locker;
    AbstractSortFilterPointer sortAndFilter { nullptr };
    FileViewFilterCallback filterCallback { nullptr };
    QVariant filterData;
    FileItemData *rootdata { nullptr };
    QString currentKey;
    Global::ItemRoles orgSortRole { Global::ItemRoles::kItemDisplayRole };
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
    DFMIO::DEnumerator::SortRoleCompareFlag sortRole { DFMIO::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    std::atomic_bool isCanceled { false };
    bool isMixDirAndFile { false };
    char placeholderMemory[4];
};

}

#endif   // FILESORTWORKER_H
