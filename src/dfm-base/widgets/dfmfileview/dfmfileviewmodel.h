/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DFMFILEVIEWMODEL_H
#define DFMFILEVIEWMODEL_H

#include "localfile/dfmlocalfileinfo.h"
#include "dfmfileviewitem.h"

#include "base/dfmschemefactory.h"
#include "base/private/dfmthreadcontainer.hpp"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QPointer>
#include <QDir>
#include <QUrl>
#include <QFuture>

#include <iostream>
#include <memory>

#include <unistd.h>

class DFMFileViewModelPrivate;
class DFMFileViewModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMFileViewModel)
public:

    explicit DFMFileViewModel(QAbstractItemView *parent = nullptr);

    virtual ~DFMFileViewModel() override {

    }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex setRootUrl(const QUrl &url);

    QUrl rootUrl();

    DAbstractFileInfoPointer fileInfo(const QModelIndex &index);

    virtual QModelIndex parent(const QModelIndex &child) const override;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private Q_SLOTS:
    void doFileDeleted(const QUrl &url){}
    void dofileAttributeChanged(const QUrl &url, const int &isExternalSource = 1){}
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl){}
    void dofileCreated(const QUrl &url){}
    void dofileModified(const QUrl &url){}
    void dofileClosed(const QUrl &url){}
    void doUpdateChildren(const QList<QSharedPointer<DFMFileViewItem>> &children);

private:
    QSharedPointer<DFMFileViewModelPrivate> d_ptr;

    // QAbstractItemModel interface
public:
    virtual void fetchMore(const QModelIndex &parent) override;

    // QAbstractItemModel interface
public:
    virtual bool canFetchMore(const QModelIndex &parent) const override;
};

#endif // DFMFILEVIEWMODEL_H
