// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESORTFILTERPROXYMODEL_H
#define FILESORTFILTERPROXYMODEL_H

#include "dfmplugin_workspace_global.h"
#include "events/workspaceeventsequence.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_global_defines.h"

#include <QSortFilterProxyModel>
#include <QDir>

namespace dfmplugin_workspace {

class FileViewModel;
class FileView;
class FileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileSortFilterProxyModel(QObject *parent = nullptr);
    virtual ~FileSortFilterProxyModel() override;

    QVariant headerData(int column, Qt::Orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex setRootUrl(const QUrl &url);
    QUrl currentRootUrl() const;

    void clear();
    void update();
    void refresh();

    void updateFile(const QUrl &url);

    AbstractFileInfoPointer itemFileInfo(const QModelIndex &index) const;

    QModelIndex getIndexByUrl(const QUrl &url) const;
    QUrl getUrlByIndex(const QModelIndex &index) const;
    QList<QUrl> getCurrentDirFileUrls() const;

    int getColumnWidth(const int &column) const;
    DFMGLOBAL_NAMESPACE::ItemRoles getRoleByColumn(const int &column) const;
    int getColumnByRole(const DFMGLOBAL_NAMESPACE::ItemRoles role) const;
    QList<DFMGLOBAL_NAMESPACE::ItemRoles> getColumnRoles() const;
    QString roleDisplayString(int role) const;

    // Filter
    QDir::Filters getFilters() const;
    QStringList getNameFilters() const;
    void setFilters(const QDir::Filters &filters);
    void setNameFilters(const QStringList &nameFilters);
    void setFilterData(const QVariant &data);
    void setFilterCallBack(const FileViewFilterCallback callback);
    void resetFilter();
    void toggleHiddenFiles();

    void setReadOnly(const bool readOnly);
    void stopWork();

    void setActive(const QModelIndex &index, bool enable = true);
    void updateRootInfo(const QList<QUrl> &urls);

    ModelState currentState() const;
    void initMixDirAndFile();

Q_SIGNALS:
    void modelChildrenUpdated();
    void stateChanged();
    void selectAndEditFile(const QUrl &url);

public Q_SLOTS:
    void onChildrenUpdate(const QUrl &url);
    void onTraverPrehandle(const QUrl &url, const QModelIndex &index, const FileView *view);
    void onStateChanged(const QUrl &url, ModelState state);
    void onSelectAndEditFile(const QUrl &rootUrl, const QUrl &url);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool passFileFilters(const AbstractFileInfoPointer &info) const;
    bool passNameFilters(const AbstractFileInfoPointer &info) const;
    bool isDefaultHiddenFile(const AbstractFileInfoPointer &info) const;

    FileViewModel *viewModel() const;

private:
    QUrl rootUrl;
    QModelIndex currentSourceIndex;

    QVariant filterData;
    FileViewFilterCallback filterCallback;
    QDir::Filters filters = QDir::NoFilter;
    QStringList nameFilters;
    mutable QMap<QUrl, bool> nameFiltersMatchResultMap;

    bool readOnly = false;
    bool isPrehandling = false;

    ModelState state = ModelState::kIdle;
    bool workStoped = false;
    bool isNotMixDirAndFile = false;
};

}

#endif   // FILESORTFILTERPROXYMODEL_H
