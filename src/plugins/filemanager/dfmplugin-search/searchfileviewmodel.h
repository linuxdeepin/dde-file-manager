/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHFILEVIEWMODEL_H
#define SEARCHFILEVIEWMODEL_H

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/widgets/dfmfileview/fileviewmodel.h"

#include <QAbstractItemView>

DFMBASE_USE_NAMESPACE

//class SearchFileViewModelPrivate;
class SearchFileViewModel : public FileViewModel
{
    Q_OBJECT
    //    QSharedPointer<SearchFileViewModelPrivate> d;
public:
    explicit SearchFileViewModel(QAbstractItemView *parent = nullptr);
    virtual ~SearchFileViewModel() override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex setRootUrl(const QUrl &url) override;
    virtual const FileViewItem *itemFromIndex(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QList<QSharedPointer<FileViewItem>> m_fileNodes;
};

#endif   // SEARCHFILEVIEWMODEL_H
