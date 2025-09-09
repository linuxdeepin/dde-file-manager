// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWMODEL_H
#define FILEVIEWMODEL_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QUrl>

#include <iostream>
#include <memory>

#include <unistd.h>

class QAbstractItemView;

namespace dfmplugin_workspace {

class FileView;
class FileItemData;
class FileSortWorker;
class RootInfo;
class GroupingManager;
class FileViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileViewModel(QAbstractItemView *parent = nullptr);
    virtual ~FileViewModel() override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int column, Qt::Orientation, int role) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QUrl rootUrl() const;
    QModelIndex rootIndex() const;

    QModelIndex setRootUrl(const QUrl &url);
    void refresh();

    void doExpand(const QModelIndex &index);
    void doCollapse(const QModelIndex &index);

    ModelState currentState() const;
    FileInfoPointer fileInfo(const QModelIndex &index) const;
    QList<QUrl> getChildrenUrls() const;
    QModelIndex getIndexByUrl(const QUrl &url) const;

    int getColumnWidth(int column) const;
    DFMGLOBAL_NAMESPACE::ItemRoles getRoleByColumn(int column) const;
    int getColumnByRole(DFMGLOBAL_NAMESPACE::ItemRoles role) const;
    QList<DFMGLOBAL_NAMESPACE::ItemRoles> getColumnRoles() const;
    DFMGLOBAL_NAMESPACE::ItemRoles columnToRole(int column) const;
    QString roleDisplayString(int role) const;

    void stopTraversWork(const QUrl &newUrl);

    void updateFile(const QUrl &url);

    Qt::SortOrder sortOrder() const;
    DFMGLOBAL_NAMESPACE::ItemRoles sortRole() const;

    void setFilters(QDir::Filters filters);
    QDir::Filters getFilters() const;

    void setNameFilters(const QStringList &filters);
    QStringList getNameFilters() const;

    void setFilterData(const QVariant &data);
    void setFilterCallback(const FileViewFilterCallback callback);

    void toggleHiddenFiles();
    void setReadOnly(bool value);
    void updateThumbnailIcon(const QModelIndex &index, const QString &thumb);
    void setTreeView(const bool isTree);

    QStringList getKeyWords();

    // 设置目录加载策略
    void setDirectoryLoadStrategy(DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy);
    DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy directoryLoadStrategy() const;

    // 预先准备URL但不加载（用于搜索等场景）
    void prepareUrl(const QUrl &url);

    // 执行实际的加载，使用之前准备的URL或当前URL
    void executeLoad();

    // Grouping management (delegated to GroupingManager)
    void setGroupingStrategy(const QString &strategyName);
    QString getGroupingStrategy() const;
    void setGroupingEnabled(bool enabled);
    bool isGroupingEnabled() const;
    void setGroupingOrder(Qt::SortOrder order);
    Qt::SortOrder getGroupingOrder() const;
    void toggleGroupExpansion(const QString &groupKey);
    bool isGroupExpanded(const QString &groupKey) const;

Q_SIGNALS:
    void stateChanged();
    void renameFileProcessStarted();
    void selectAndEditFile(const QUrl &url);
    void traverPrehandle(const QUrl &url, const QModelIndex &index, FileView *view);

    void hiddenFileChanged();
    void filtersChanged(QStringList nameFilters, QDir::Filters filters);

    void requestChangeHiddenFilter();
    void requestChangeFilters(QDir::Filters filters);
    void requestChangeNameFilters(const QStringList &nameFilters);
    void requestUpdateFile(const QUrl &url);
    void requestRefreshAllChildren();
    void requestClearThumbnail();

    void requestSortChildren(Qt::SortOrder order, DFMGLOBAL_NAMESPACE::ItemRoles role, const bool isMixAndFile);
    void requestGroupChildren(Qt::SortOrder order, DFMGLOBAL_NAMESPACE::ItemRoles role);
    void requestSetFilterData(const QVariant &data);
    void requestSetFilterCallback(FileViewFilterCallback callback);
    void requestShowHiddenChanged(bool value);

    void requestCollapseItem(const QString &key, const QUrl &parent);
    void requestTreeView(const bool isTree);

    // Grouping-related signals
    void groupingStrategyChanged(const QString &strategyName);
    void groupingEnabledChanged(bool enabled);
    void groupingOrderChanged(Qt::SortOrder order);
    void groupExpansionChanged(const QString &groupKey, bool expanded);

    void aboutToSwitchToListView(const QList<QUrl> &allShowList);

public Q_SLOTS:
    void onFileThumbUpdated(const QUrl &url, const QString &thumb);
    void onFileUpdated(int show);
    void onInsert(int firstIndex, int count);
    void onInsertFinish();
    void onRemove(int firstIndex, int count);
    void onRemoveFinish();
    void onUpdateView();
    void onGenericAttributeChanged(DFMBASE_NAMESPACE::Application::GenericAttribute ga, const QVariant &value);
    void onDConfigChanged(const QString &config, const QString &key);
    void onSetCursorWait();
    void onHiddenSettingChanged(bool value);
    void onWorkFinish(int visiableCount, int totalCount);
    void onDataChanged(int first, int last);
    void onGroupingDataChanged();

private:
    void connectRootAndFilterSortWork(RootInfo *root, const bool refresh = false);
    void connectFilterSortWorkSignals();
    void initFilterSortWork();
    void quitFilterSortWork();
    void discardFilterSortObjects();

    void changeState(ModelState newState);
    void closeCursorTimer();
    void startCursorTimer();

    // Grouping management helper
    void initializeGroupingManager();

    QUrl dirRootUrl;
    QUrl fetchingUrl;

    ModelState state { ModelState::kIdle };
    bool readOnly { false };
    bool canFetchFiles { false };
    FileItemData *itemRootData { nullptr };

    QSharedPointer<QThread> filterSortThread { nullptr };
    QSharedPointer<FileSortWorker> filterSortWorker { nullptr };
    FileViewFilterCallback filterCallback { nullptr };
    QVariant filterData;
    QString currentKey;
    QTimer waitTimer;

    QList<QSharedPointer<QObject>> discardedObjects {};
    QDir::Filters currentFilters { QDir::NoFilter };
    QStringList nameFilters {};
    bool isTree { false };

    // Grouping management
    std::unique_ptr<GroupingManager> m_groupingManager;

    DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy dirLoadStrategy { DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy::kCreateNew };
    QUrl preparedUrl;
};

}

#endif   // FILEVIEWMODEL_H
