/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#ifndef ITEMDELEGATEHELPER_H
#define ITEMDELEGATEHELPER_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/utils/elidetextlayout.h"

#include <QStyledItemDelegate>
#include <QTextLayout>

namespace dfmplugin_workspace {

// begin file view item icon delegate global define
inline constexpr int kIconModeTextPadding = { 4 };
inline constexpr int kIconModeIconSpacing = { 5 };
inline constexpr int kIconModeRectRadius = kIconModeTextPadding;
inline constexpr int kIconModeBackRadius = { 18 };
inline constexpr int kIconModeColumuPadding { 10 };
// end

// begin file view item list delegate global define
inline constexpr int kListModeRectRadius = { 8 };
inline constexpr int kListModeLeftPadding = { 10 };
inline constexpr int kListModeIconSpacing = { 6 };
inline constexpr int kListModeRightPadding = { 10 };
inline constexpr int kListModeColumnPadding = { 10 };
// end

class ItemDelegateHelper
{
public:
    static inline Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
    {
        if (!(alignment & Qt::AlignHorizontal_Mask))
            alignment |= Qt::AlignLeft;
        if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
            if (direction == Qt::RightToLeft)
                alignment ^= (Qt::AlignLeft | Qt::AlignRight);
            alignment |= Qt::AlignAbsolute;
        }
        return alignment;
    }
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static void paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect,
                          Qt::Alignment alignment = Qt::AlignCenter,
                          QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    static void hideTooltipImmediately();

    static dfmbase::ElideTextLayout *createTextLayout(const QString &name, QTextOption::WrapMode wordWrap,
                                                      qreal lineHeight, int alignmentFlag, QPainter *painter = nullptr);

private:
    static void drawBackground(const qreal &backgroundRadius, const QRectF &rect,
                               QRectF &lastLineRect, const QBrush &backgroundBrush, QPainter *painter);
};

}

#endif   // ITEMDELEGATEHELPER_H
