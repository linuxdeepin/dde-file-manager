/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include "ddiskmanager.h"

#include <QAbstractItemModel>

#include "durl.h"
#include "interfaces/dabstractfileinfo.h"
#include "views/progressline.h"

class ComputerView2;

struct ComputerModelItemData
{
    enum StorageDataSource
    {
        ds_gio,
        ds_udisks,
    };
    enum Category
    {
        cat_user_directory,
        cat_internal_storage,
        cat_external_storage,
        cat_splitter,
        cat_widget
    };
    DAbstractFileInfoPointer fi;
    QSharedPointer<DBlockDevice> blkdev;
    QList<DFMGlobal::MenuAction> menuactionlist;
    DUrl url;
    QString sptext;
    QWidget* widget = nullptr;
    ProgressLine* pl = nullptr;
    StorageDataSource ds;
    Category cat;
    bool selected;
};

class ComputerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum DataRoles
    {
        SizeTotalRole = Qt::UserRole + 1,  //uint64_t
        SizeInUseRole = Qt::UserRole + 2,  //uint64_t
        FileSystemRole = Qt::UserRole + 3, //QString
        UsgWidgetRole = Qt::UserRole + 4,  //ProgressLine*
        ICategoryRole = Qt::UserRole + 5,  //ComputerModelItemData::Category
    };
    explicit ComputerModel(QObject* parent = nullptr);
    ~ComputerModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
public Q_SLOTS:
    void addItem(const DUrl &url, QWidget *w = nullptr);
    void insertAfter(const DUrl &url, const DUrl &ref, QWidget *w = nullptr);
    void insertBefore(const DUrl &url, const DUrl &ref, QWidget *w = nullptr);
    void removeItem(const DUrl &url);

private:
    ComputerView2* par;
    QScopedPointer<DDiskManager> m_diskm;
    QList<ComputerModelItemData> m_items;
};


#endif // COMPUTERMODEL_H
