// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QPixmap>

DFMBASE_BEGIN_NAMESPACE

namespace IconUtils {
QPixmap renderIconBackground(const QSize &size);
QPixmap addShadowToPixmap(const QPixmap &originalPixmap, int shadowOffsetY, qreal blurRadius, qreal shadowOpacity);
}   // end namespace IconUtils

DFMBASE_END_NAMESPACE

#endif   // ICONUTILS_H
