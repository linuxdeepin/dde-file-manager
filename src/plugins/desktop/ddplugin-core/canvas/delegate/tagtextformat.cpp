/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "tagtextformat.h"

TagTextFormat::TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor)
{
    setObjectType(objectType);
    setProperty(QTextFormat::UserProperty + 1, QVariant::fromValue(colors));
    setProperty(QTextFormat::UserProperty + 2, borderColor);
}

QList<QColor> TagTextFormat::colors() const
{
    return qvariant_cast<QList<QColor>>(property(QTextFormat::UserProperty + 1));
}

QColor TagTextFormat::borderColor() const
{
    return colorProperty(QTextFormat::UserProperty + 2);
}

qreal TagTextFormat::diameter() const
{
    return 12; // TODO(Lee): why 12 ?
}
