// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESORTWORKER_H
#define FILESORTWORKER_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"
#include "groupingengine.h"
#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include "groupedmodeldata.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <dfm-base/base/application/application.h>

#include <dfm-io/denumerator.h>

#include <QObject>
#include <QDirIterator>
#include <QReadWriteLock>
#include <QMultiMap>

using namespace dfmbase;
namespace dfmplugin_workspace {
class FileSortWorker : public QObject
{
    Q_OBJECT
    enum class SortOpt : uint8_t {
        kSortOptNone = 0,
        kSortOptOnlyOrderChanged = 1,
        kSortOptOtherChanged = 2,
    };

    enum class InsertOpt : uint8_t {
        kInsertOptAppend = 0,
        kInsertOptReplace = 1,
        kInsertOptForce = 2,
    };

    enum class SortScenarios : uint8_t {
        kSortScenariosIteratorAddFile = 1,   // Iterator adding new files for sorting
        kSortScenariosIteratorExistingFile = 2,   // Iterator handling existing files in display model
        kSortScenariosNormal = 3,   // Normal display completion scenario
        kSortScenariosWatcherAddFile = 4,   // File watcher detected file addition (including rename)
        kSortScenariosWatcherOther = 5,   // Other file watcher scenarios
    };

public:
    explicit FileSortWorker(const QUrl &url,
                            const QString &key,
                            FileViewFilterCallback callfun = nullptr,
                            const QStringList &nameFilters = QStringList(),
                            const QDir::Filters filters = QDir::NoFilter,
                            QObject *parent = nullptr);
    ~FileSortWorker();
    SortOpt setSortAgruments(const Qt::SortOrder order, const dfmbase::Global::ItemRoles sortRole,
                             const bool isMixDirAndFile);
    // Grouping-related methods
    void setGroupingStrategy(std::unique_ptr<AbstractGroupStrategy> strategy);
    void setGroupingEnabled(bool enabled);
    bool isGroupingEnabled() const;
    void setGroupOrder(Qt::SortOrder order);
    Qt::SortOrder getGroupOrder() const;
    void toggleGroupExpansion(const QString &groupKey);
    bool isGroupExpanded(const QString &groupKey) const;
    GroupedModelData getGroupedModelData() const;

    int childrenCount();
    QVariant groupHeaderData(const int index, const int role);
    FileItemDataPointer childData(const int index);
    FileItemDataPointer childData(const QUrl &url);
    void setRootData(const FileItemDataPointer data);
    FileItemDataPointer rootData() const;
    void cancel();
    int getChildShowIndex(const QUrl &url);
    QList<QUrl> getChildrenUrls();

    DFMGLOBAL_NAMESPACE::ItemRoles getSortRole() const;
    Qt::SortOrder getSortOrder() const;

    // 只有在没有启动sort线程时才能调用，线程启动成功了，发送信号处理
    void setTreeView(const bool isTree);

signals:
    void insertRows(int first, int count);
    void insertFinish();
    void removeRows(int first, int count);
    void removeFinish();
    void requestFetchMore();
    void selectAndEditFile(const QUrl &url);
    void dataChanged(int first, int last);

    void requestSetIdel(int visiableCount, int totalCount);
    void updateRow(int row);

    // notify data
    void getSourceData(const QString &key);

    void requestUpdateView();
    void requestCursorWait();
    void reqUestCloseCursor();

    // Grouping-related signals
    void groupingDataChanged();

    // Note that the slot functions here are executed in asynchronous threads,
    // so the link can only be Qt:: QueuedConnection,
    // which cannot be directly called elsewhere, but can only be triggered by signals
signals:
    void requestSortByMimeType();
    void aboutToSwitchToListView(const QList<QUrl> &allShowList);

public slots:
    // Receive all local files of iteration of iterator thread, perform filtering and sorting
    void handleIteratorLocalChildren(const QString &key,
                                     const QList<SortInfoPointer> children,
                                     const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                                     const Qt::SortOrder sortOrder,
                                     const bool isMixDirAndFile,
                                     bool isFirstBatch = false);
    void handleSourceChildren(const QString &key,
                              const QList<SortInfoPointer> children,
                              const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                              const Qt::SortOrder sortOrder,
                              const bool isMixDirAndFile,
                              const bool isFinished);
    void handleIteratorChildren(const QString &key, const QList<SortInfoPointer> children, const QList<FileInfoPointer> infos, bool isFirstBatch = false);
    void handleIteratorChildrenUpdate(const QString &key, const QList<SortInfoPointer> children, bool isFirstBatch = false);
    void handleTraversalFinish(const QString &key, bool noDataProduced = false);
    void handleSortDir(const QString &key, const QUrl &parent);

    void handleFilters(QDir::Filters filters);
    void HandleNameFilters(const QStringList &filters);
    void handleFilterData(const QVariant &data);
    void handleFilterCallFunc(FileViewFilterCallback callback);

    void onToggleHiddenFiles();
    void onShowHiddenFileChanged(bool isShow);

    void handleWatcherAddChildren(const QList<SortInfoPointer> &children);
    void handleWatcherRemoveChildren(const QList<SortInfoPointer> &children);
    bool handleWatcherUpdateFile(const SortInfoPointer child);
    void handleWatcherUpdateFiles(const QList<SortInfoPointer> &children);
    void handleWatcherUpdateHideFile(const QUrl &hidUrl);

    void handleResort(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile);
    void performGrouping();   // Modern grouping execution method
    void onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value);

    bool handleUpdateFile(const QUrl &url);
    void handleUpdateFiles(const QList<QUrl> &urls);

    void handleRefresh();
    void handleClearThumbnail();
    void handleFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg);
    void handleUpdateRefreshFiles();
    void handleSortByMimeType();

    // treeview solts
public slots:
    void handleCloseExpand(const QString &key, const QUrl &parent);
    // 如果不是tree视图，切换到tree视图，就去执行处理dir是否可以展开属性设置
    // 如果是tree视图，切换到普通试图，去掉所有子目录，去掉所有的是否可以展开属性
    void handleSwitchTreeView(const bool isTree);

private:
    void handleAddChildren(const QString &key,
                           QList<SortInfoPointer> children,
                           const QList<FileInfoPointer> &childInfos,
                           const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                           const Qt::SortOrder sortOrder,
                           const bool isMixDirAndFile,
                           const bool handleSource,
                           const bool isFinished, const bool isSort = true,
                           const bool isFirstBatch = false);
    bool handleAddChildren(const QString &key,
                           const QList<SortInfoPointer> &children,
                           const QList<FileInfoPointer> &childInfos,
                           const bool isFirstBatch = false);
    void setSourceHandleState(const bool isFinished);
    void resetFilters(const QDir::Filters filters = QDir::NoFilter);
    void checkNameFilters(const FileItemDataPointer itemData);
    void filterAllFilesOrdered();
    void filterAndSortFiles(const QUrl &dir, const bool fileter = false,
                            const bool reverse = false);
    void resortCurrent(const bool reverse);
    QList<QUrl> filterFilesByParent(const QUrl &dir, const bool byInfo = false);
    void filterTreeDirFiles(const QUrl &parent, const bool byInfo = false);

    bool addChild(const SortInfoPointer &sortInfo,
                  const SortScenarios sort);
    bool sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo);

    void switchTreeView();
    void switchListView();
    QList<QUrl> sortAllTreeFilesByParent(const QUrl &dir, const bool reverse = false);
    QList<QUrl> sortTreeFiles(const QList<QUrl> &children, const bool reverse = false);
    QList<QUrl> removeChildrenByParents(const QList<QUrl> &dirs);
    QList<QUrl> removeVisibleTreeChildren(const QUrl &parent);
    void removeSubDir(const QUrl &dir);
    void removeFileItems(const QList<QUrl> &urls);
    int8_t findDepth(const QUrl &parent);
    int findEndPos(const QUrl &dir);
    int findStartPos(const QUrl &parent);
    int findStartPos(const QList<QUrl> &list, const QUrl &parent);

    void resortVisibleChildren(const QList<QUrl> &fileUrls);
    void insertVisibleChildren(const int startPos, const QList<QUrl> &filterUrls,
                               const InsertOpt opt = InsertOpt::kInsertOptAppend, const int endPos = -1);
    void removeVisibleChildren(const int startPos, const int size);
    void createAndInsertItemData(const int8_t depth, const SortInfoPointer child, const FileInfoPointer info);

    int insertSortList(const QUrl &needNode, const QList<QUrl> &list,
                       SortScenarios sort);
    bool lessThan(const QUrl &left, const QUrl &right, SortScenarios sort);
    QVariant data(const FileInfoPointer &info, Global::ItemRoles role);
    QVariant data(const SortInfoPointer &info, Global::ItemRoles role);

    bool checkFilters(const SortInfoPointer &sortInfo, const bool byInfo = false);
    bool isDefaultHiddenFile(const QUrl &fileUrl);
    QUrl makeParentUrl(const QUrl &url);
    int8_t getDepth(const QUrl &url);
    int findRealShowIndex(const QUrl &preItemUrl);
    int indexOfVisibleChild(const QUrl &itemUrl);
    int setVisibleChildren(const int startPos, const QList<QUrl> &filterUrls,
                           const InsertOpt opt = InsertOpt::kInsertOptAppend, const int endPos = -1);
    bool checkAndUpdateFileInfoUpdate();
    void checkAndSortBytMimeType(const QUrl &url);
    void doCompleteFileInfo(SortInfoPointer sortInfo);

    // Grouping-related private methods
    QList<FileItemDataPointer> getAllFiles() const;
    FileItemDataPointer createGroupHeaderData(const FileGroupData *groupData) const;

private:
    QUrl current;
    QStringList nameFilters {};
    QDir::Filters filters { QDir::NoFilter };
    QHash<QUrl, QHash<QUrl, SortInfoPointer>> children {};
    mutable QReadWriteLock childrenDataLocker;
    QHash<QUrl, FileItemDataPointer> childrenDataMap {};
    QList<QUrl> visibleChildren {};
    QReadWriteLock locker;
    FileViewFilterCallback filterCallback { nullptr };
    QVariant filterData;
    FileItemDataPointer rootdata { nullptr };
    QString currentKey;
    Global::ItemRoles orgSortRole { Global::ItemRoles::kItemDisplayRole };
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
    DFMIO::DEnumerator::SortRoleCompareFlag sortRole { DFMIO::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };

    // Group-by configuration
    Qt::SortOrder groupOrder { Qt::AscendingOrder };

    // New grouping-related members
    std::unique_ptr<GroupingEngine> groupingEngine;
    std::unique_ptr<AbstractGroupStrategy> currentStrategy;
    GroupedModelData groupedData;
    std::atomic_bool m_isGroupingEnabled { false };
    QHash<QString, bool> groupExpansionStates;

    std::atomic_bool isCanceled { false };
    bool isMixDirAndFile { false };
    QHash<QUrl, QList<QUrl>> visibleTreeChildren {};
    QMultiMap<int8_t, QUrl> depthMap;
    std::atomic_bool istree { false };
    std::atomic_bool currentSupportTreeView { false };
    QList<QUrl> fileInfoRefresh;
    QTimer *updateRefresh { nullptr };
    std::atomic_bool mimeSorting { false };
    QSet<QUrl> waitUpdatedFiles;
};

}

#endif   // FILESORTWORKER_H
