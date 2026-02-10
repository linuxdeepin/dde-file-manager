// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGTEXTFORMAT_H
#define TAGTEXTFORMAT_H

#include "dfmplugin_tag_global.h"

#include <QTextCharFormat>

namespace dfmplugin_tag {

class TagTextFormat : public QTextCharFormat
{
public:
    TagTextFormat(int objectType, const QList<QColor> &colors, const QColor &borderColor);

    QList<QColor> colors() const;
    QColor borderColor() const;
    qreal diameter() const;
};

}

#endif   // TAGTEXTFORMAT_H
