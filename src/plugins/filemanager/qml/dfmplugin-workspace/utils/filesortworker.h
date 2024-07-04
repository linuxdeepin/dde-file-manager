// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESORTWORKER_H
#define FILESORTWORKER_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractsortfilter.h>
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
    FileItemDataPointer childData(const int index);
    FileItemDataPointer childData(const QUrl &url);
    void setRootData(const FileItemDataPointer data);
    FileItemDataPointer rootData() const;
    void cancel();
    int getChildShowIndex(const QUrl &url);
    QList<QUrl> getChildrenUrls();

    QDir::Filters getFilters() const;

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

    // Note that the slot functions here are executed in asynchronous threads,
    // so the link can only be Qt:: QueuedConnection,
    // which cannot be directly called elsewhere, but can only be triggered by signals
signals:
    void requestUpdateTimerStart();

public slots:
    // Receive all local files of iteration of iterator thread, perform filtering and sorting
    void handleIteratorLocalChildren(const QString &key,
                                     const QList<SortInfoPointer> children,
                                     const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                                     const Qt::SortOrder sortOrder,
                                     const bool isMixDirAndFile);
    void handleSourceChildren(const QString &key,
                              const QList<SortInfoPointer> children,
                              const DFMIO::DEnumerator::SortRoleCompareFlag sortRole,
                              const Qt::SortOrder sortOrder,
                              const bool isMixDirAndFile,
                              const bool isFinished);
    void handleIteratorChildren(const QString &key,const QList<SortInfoPointer> children, const QList<FileInfoPointer> infos);
    void handleTraversalFinish(const QString &key);
    void handleSortDir(const QString &key, const QUrl &parent);

    // Get data from the data area according to the url, filter and sort the data
    void handleModelGetSourceData();
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
    void onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value);

    bool handleUpdateFile(const QUrl &url);
    void handleUpdateFiles(const QList<QUrl> &urls);

    void handleRefresh();
    void handleClearThumbnail();
    void handleFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg);
    void handleUpdateRefreshFiles();

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
                           const bool isFinished, const bool isSort = true);
    bool handleAddChildren(const QString &key,
                           const QList<SortInfoPointer> &children,
                           const QList<FileInfoPointer> &childInfos);
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
                  const AbstractSortFilter::SortScenarios sort);
    bool sortInfoUpdateByFileInfo(const FileInfoPointer fileInfo);

private:
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

private:
    int insertSortList(const QUrl &needNode, const QList<QUrl> &list,
                       AbstractSortFilter::SortScenarios sort);
    bool lessThan(const QUrl &left, const QUrl &right, AbstractSortFilter::SortScenarios sort);
    QVariant data(const FileInfoPointer &info, Global::ItemRoles role);

    bool checkFilters(const SortInfoPointer &sortInfo, const bool byInfo = false);
    bool isDefaultHiddenFile(const QUrl &fileUrl);
    QUrl parantUrl(const QUrl &url);
    int8_t getDepth(const QUrl &url);
    int findRealShowIndex(const QUrl &preItemUrl);
    int indexOfVisibleChild(const QUrl &itemUrl);
    int setVisibleChildren(const int startPos, const QList<QUrl> &filterUrls,
                            const InsertOpt opt = InsertOpt::kInsertOptAppend, const int endPos = -1);

private:
    QUrl current;
    QStringList nameFilters {};
    QDir::Filters filters { QDir::NoFilter };
    QDirIterator::IteratorFlags flags { QDirIterator::NoIteratorFlags };
    QMap<QUrl, QMap<QUrl, SortInfoPointer>> children {};
    QReadWriteLock childrenDataLocker;
    QMap<QUrl, FileItemDataPointer> childrenDataMap {};
    QMap<QUrl, FileItemDataPointer> childrenDataLastMap {};
    QList<QUrl> visibleChildren {};
    QReadWriteLock locker;
    AbstractSortFilterPointer sortAndFilter { nullptr };
    FileViewFilterCallback filterCallback { nullptr };
    QVariant filterData;
    FileItemDataPointer rootdata { nullptr };
    QString currentKey;
    Global::ItemRoles orgSortRole { Global::ItemRoles::kItemDisplayRole };
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
    DFMIO::DEnumerator::SortRoleCompareFlag sortRole { DFMIO::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    std::atomic_bool isCanceled { false };
    bool isMixDirAndFile { false };
    char placeholderMemory[4];
    QMap<QUrl, QList<QUrl>> visibleTreeChildren{};
    QMultiMap<int8_t, QUrl> depthMap;
    std::atomic_bool istree;
    std::atomic_bool currentSupportTreeView {false};
    QList<QUrl> fileInfoRefresh;
    QTimer *updateRefresh {nullptr};
};

}

#endif   // FILESORTWORKER_H
