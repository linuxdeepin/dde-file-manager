/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef CANVASITEMDELEGATE_P_H
#define CANVASITEMDELEGATE_P_H

#include "canvasitemdelegate.h"
#include "elidetextlayout.h"

#include <QPointer>
#include <QTextDocument>
#include <QAbstractItemView>

namespace ddplugin_canvas {

class CanvasItemDelegatePrivate
{
public:
    explicit CanvasItemDelegatePrivate(CanvasItemDelegate *qq);
    ~CanvasItemDelegatePrivate();

    ElideTextLayout *createTextlayout(const QModelIndex &index, const QPainter *painter = nullptr) const;

    inline QRect availableTextRect(QRect labelRect) const
    {
        // available text rect top is label rect minus icon space and text padding.
        labelRect.setTop(labelRect.top() + CanvasItemDelegate::kIconSpacing
                         + CanvasItemDelegate::kTextPadding);
        return labelRect;
    }
    inline bool isHighlight(const QStyleOptionViewItem &option) const
    {
        return (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    }

    bool needExpend(const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QRect &rText, QRect *needText = nullptr) const;

public:
    // default icon size is 48px.
    int currentIconLevel = -1;
    int textLineHeight = -1;
    QList<int> iconSizes;
    QSize itemSizeHint;

    QTextDocument *document { nullptr };
    CanvasItemDelegate *const q;
};

}

#endif   // CANVASITEMDELEGATE_P_H
