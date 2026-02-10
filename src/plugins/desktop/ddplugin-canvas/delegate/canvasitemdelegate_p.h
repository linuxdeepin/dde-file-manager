// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASITEMDELEGATE_P_H
#define CANVASITEMDELEGATE_P_H

#include "canvasitemdelegate.h"

#include <dfm-base/utils/elidetextlayout.h>

#include <QPointer>
#include <QTextDocument>
#include <QAbstractItemView>

namespace ddplugin_canvas {

class CanvasItemDelegatePrivate
{
public:
    explicit CanvasItemDelegatePrivate(CanvasItemDelegate *qq);
    ~CanvasItemDelegatePrivate();

    dfmbase::ElideTextLayout *createTextlayout(const QModelIndex &index, const QPainter *painter = nullptr) const;

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

    static void extendLayoutText(const FileInfoPointer &info,  dfmbase::ElideTextLayout *layout);
public:
    CanvasItemDelegate *const q = nullptr;
    // default icon size is 48px.
    int currentIconLevel = -1;
    int textLineHeight = -1;
    QList<int> iconSizes;
    QSize itemSizeHint;

    QTextDocument *document { nullptr };
};

}

#endif   // CANVASITEMDELEGATE_P_H
