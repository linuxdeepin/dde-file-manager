// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPLETERVIEWDELEGATE_H
#define COMPLETERVIEWDELEGATE_H

#include "dfmplugin_titlebar_global.h"

#include <DStyledItemDelegate>

namespace dfmplugin_titlebar {

class CompleterViewDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CompleterViewDelegate(QAbstractItemView *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    void paintItemIcon(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    QPixmap createCustomOpacityPixmap(const QPixmap &px, float opacity) const;
};

}

#endif   // COMPLETERVIEWDELEGATE_H
