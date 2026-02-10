// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPAINTER_H
#define TAGPAINTER_H

#include "dfmplugin_tag_global.h"

#include <QTextObjectInterface>

namespace dfmplugin_tag {

class TagPainter : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    explicit TagPainter();

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override;

    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override;
};

}

#endif   // TAGPAINTER_H
