// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QPixmap>

DFMBASE_BEGIN_NAMESPACE

namespace IconUtils {
struct IconStyle
{
    int stroke { 2 };
    int radius { 4 };
    int shadowOffset { 1 };
    int shadowRange { 3 };
};
QPixmap renderIconBackground(const QSize &size, const IconStyle &style = IconStyle {});
QPixmap renderIconBackground(const QSizeF &size, const IconStyle &style = IconStyle {});
QPixmap addShadowToPixmap(const QPixmap &originalPixmap, int shadowOffsetY, qreal blurRadius, qreal shadowOpacity);
IconStyle getIconStyle(int size);
}   // end namespace IconUtils

DFMBASE_END_NAMESPACE

#endif   // ICONUTILS_H
