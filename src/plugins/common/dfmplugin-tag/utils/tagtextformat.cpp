// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagtextformat.h"

DPTAG_USE_NAMESPACE

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
    return kTagDiameter;
}
