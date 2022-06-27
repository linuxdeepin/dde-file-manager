/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILESORTFILTERPROXYMODEL_H
#define FILESORTFILTERPROXYMODEL_H

#include "dfmplugin_workspace_global.h"
#include "workspace/workspace_defines.h"
#include "views/fileviewitem.h"
#include "events/workspaceeventsequence.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_global_defines.h"

#include <QSortFilterProxyModel>
#include <QDir>

DSB_FM_USE_NAMESPACE

using namespace Workspace;
namespace dfmplugin_workspace {

class FileViewModel;
class FileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileSortFilterProxyModel(QObject *parent = nullptr);
    virtual ~FileSortFilterProxyModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int column, Qt::Orientation, int role) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex setRootUrl(const QUrl &url);

    QUrl rootUrl() const;
    QModelIndex rootIndex() const;
    const FileViewItem *rootItem() const;

    const FileViewItem *itemFromIndex(const QModelIndex &index) const;
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
    void setFilters(const QDir::Filters &filters);
    void setNameFilters(const QStringList &nameFilters);
    void setFilterData(const QVariant &data);
    void setFilterCallBack(const FileViewFilterCallback callback);
    void resetFilter();
    void toggleHiddenFiles();

    void setReadOnly(const bool readOnly);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool passFileFilters(const AbstractFileInfoPointer &info) const;
    bool passNameFilters(const AbstractFileInfoPointer &info) const;

    FileViewModel *viewModel() const;

private:
    QVariant filterData;
    FileViewFilterCallback filterCallback;
    QDir::Filters filters = QDir::NoFilter;
    QStringList nameFilters;
    mutable QMap<QUrl, bool> nameFiltersMatchResultMap;

    bool readOnly = false;
};

}

#endif   // FILESORTFILTERPROXYMODEL_H
