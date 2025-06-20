// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERVIEWDELEGATE_H
#define FOLDERVIEWDELEGATE_H

#include "dfmplugin_titlebar_global.h"

#include <DStyledItemDelegate>

namespace dfmplugin_titlebar {

class FolderViewDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FolderViewDelegate(QAbstractItemView *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void paintItemIcon(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    QPixmap createCustomOpacityPixmap(const QPixmap &px, float opacity) const;
    void hideTooltipImmediately();
};

}

#endif   // COMPLETERVIEWDELEGATE_H
