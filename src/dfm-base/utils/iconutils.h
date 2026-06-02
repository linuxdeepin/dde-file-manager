// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QIcon>
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
bool shouldSkipThumbnailFrame(const QString &mimeType);

/*!
 * \brief Generate a HiDPI-aware pixmap from QIcon.
 * Returns a pixmap scaled by the given widget's devicePixelRatio,
 * so it renders crisply on high-DPI screens.
 * \param icon Source QIcon
 * \param size Logical pixel size (e.g. 128x128)
 * \param widget Target widget to read devicePixelRatio from
 */
QPixmap hiDpiPixmap(const QIcon &icon, const QSize &size, const QWidget *widget);
}   // end namespace IconUtils

DFMBASE_END_NAMESPACE

#endif   // ICONUTILS_H
