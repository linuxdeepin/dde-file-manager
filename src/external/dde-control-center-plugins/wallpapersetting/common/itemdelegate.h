// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QAbstractItemDelegate>

namespace dfm_wallpapersetting {

class ListView;

class ItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegate(ListView *parent = nullptr);
    ListView *view() const;
    void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;
};

}
#endif // ITEMDELEGATE_H
