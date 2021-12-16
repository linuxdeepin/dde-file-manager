/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#ifndef FILEVIEWMODEL_H
#define FILEVIEWMODEL_H

#include "fileviewitem.h"

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QUrl>

#include <iostream>
#include <memory>

#include <unistd.h>

class QAbstractItemView;
class FileViewModelPrivate;
class FileViewModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class FileViewModelPrivate;
    QSharedPointer<FileViewModelPrivate> d;

public:
    explicit FileViewModel(QAbstractItemView *parent = nullptr);
    virtual ~FileViewModel() override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual const FileViewItem *itemFromIndex(const QModelIndex &index) const;
    virtual QModelIndex setRootUrl(const QUrl &url);
    virtual QUrl rootUrl() const;
    virtual AbstractFileInfoPointer fileInfo(const QModelIndex &index);
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual void clear();
    virtual int rowCountMaxShow();
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual QVariant headerData(int column, Qt::Orientation, int role) const override;

    virtual void updateViewItem(const QModelIndex &index);

    int getColumnWidth(const int &column) const;
    FileViewItem::Roles getRoleByColumn(const int &column) const;
};

#endif   // FILEVIEWMODEL_H
