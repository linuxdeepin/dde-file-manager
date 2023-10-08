// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "commondefine.h"

#include <DIconButton>

#include <QListView>

namespace dfm_wallpapersetting {

class ItemDelegate;
class ItemModel;

class ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = nullptr);
    void initialize();
    ItemModel *itemModel() const;
    void setShowDelete(bool enable);
protected:
    void updateButton(const QModelIndex &index);
    bool viewportEvent(QEvent *event) override;
signals:
    void deleteButtonClicked(const ItemNodePtr &ptr);
    void itemClicked(const ItemNodePtr &ptr);
protected slots:
    void onItemClicked(const QModelIndex &index);
    void onButtonClicked();
    void onItemRemoved(const QModelIndex &parent, int first, int last);
private:
    ItemDelegate *delegate = nullptr;
    bool enableDelete = false;
    DTK_WIDGET_NAMESPACE::DIconButton *deleteBtn = nullptr;
};

}

#endif // LISTVIEW_H
