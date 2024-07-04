// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONDELEGATE_H
#define COLLECTIONDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringListModel>
#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class CollectionDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    CollectionDelegate(QAbstractItemView *parent = nullptr);
    virtual ~CollectionDelegate() override;
    virtual QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
signals:
    void removeItemManually(const QString &text, int row);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class CollectionModel : public QStringListModel
{
    Q_OBJECT

    // QAbstractItemModel interface
public:
    explicit CollectionModel(QObject *parent)
        : QStringListModel(parent) {}

    enum {
        kUrlRole = Qt::UserRole + 1,
    };
    virtual QVariant data(const QModelIndex &index, int role) const override;
    int findItem(const QString &item);
};

#endif   // COLLECTIONDELEGATE_H
