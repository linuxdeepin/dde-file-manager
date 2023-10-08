// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include "commondefine.h"

#include <QAbstractItemModel>

namespace dfm_wallpapersetting {

class ItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ItemModel(QObject *parent = nullptr);
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void insertItem(int pos, ItemNodePtr it);
    void removeItem(const QString &item);
    ItemNodePtr itemNode(const QModelIndex &idx) const;
    QModelIndex itemIndex(const QString &item) const;
    void resetData(const QList<ItemNodePtr> &list);
protected:
    QList<ItemNodePtr> items;
};

}

#endif // ITEMMODEL_H
