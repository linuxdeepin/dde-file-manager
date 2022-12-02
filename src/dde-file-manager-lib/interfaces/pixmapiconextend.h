// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PIXMAPICONEXTEN_H
#define PIXMAPICONEXTEN_H

#include <QIcon>

class PixmapIconExtend : public QIcon
{
public:
    explicit PixmapIconExtend(const QIcon &other);
    virtual ~PixmapIconExtend();
    QPixmap pixmapExtend(const QSize &size, qreal devicePixelRatio, Mode mode = Normal, State state = Off);

private:
    qreal pixmapDevicePixelRatio(qreal displayDevicePixelRatio, const QSize &requestedSize, const QSize &actualSize);
};

#endif   // PIXMAPICONEXTEN_H
