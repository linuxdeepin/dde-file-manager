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

#include <QSortFilterProxyModel>

DSB_FM_USE_NAMESPACE
using namespace Workspace;
DPWORKSPACE_BEGIN_NAMESPACE

class FileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileSortFilterProxyModel(QObject *parent = nullptr);
    virtual ~FileSortFilterProxyModel() override;

    void setFilterData(const QVariant &data);
    void setFilterCallBack(const FileViewFilterCallback callback);
    void resetFilter();

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QVariant filterData;
    FileViewFilterCallback filterCallback;
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILESORTFILTERPROXYMODEL_H
