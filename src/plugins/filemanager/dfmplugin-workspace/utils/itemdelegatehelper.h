// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMDELEGATEHELPER_H
#define ITEMDELEGATEHELPER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/utils/iconcachemanager.h>
#include <dfm-base/utils/iconpainterutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <QStyledItemDelegate>
#include <QTextLayout>

namespace dfmplugin_workspace {

// begin file view item icon delegate global define
inline constexpr int kIconModeTextPadding = { 4 };   // 选中背景和文字之间的距离
inline constexpr int kIconModeIconSpacing = { 3 };   // icon与背景的边距

inline constexpr int kIconModeRectRadius = kIconModeTextPadding;
inline constexpr int kIconModeBackRadiusCoefficient = { 16 };
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
    using PaintIconOpts = DFMBASE_NAMESPACE::IconPainterUtils::PaintIconOpts;
    static bool paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts)
    {
        return DFMBASE_NAMESPACE::IconPainterUtils::paintIcon(painter, icon, opts).has_value();
    }

    static void hideTooltipImmediately();

    static dfmbase::ElideTextLayout *createTextLayout(const QString &name, QTextOption::WrapMode wordWrap,
                                                      qreal lineHeight, int alignmentFlag, QPainter *painter = nullptr);

private:
    static void drawBackground(const qreal &backgroundRadius, const QRectF &rect,
                               QRectF &lastLineRect, const QBrush &backgroundBrush, QPainter *painter);
};

}

#endif   // ITEMDELEGATEHELPER_H
