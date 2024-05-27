// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SURFACE_H
#define SURFACE_H

#include "ddplugin_organizer_global.h"

#include <QWidget>

namespace ddplugin_organizer {

class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = nullptr);
    static int cellWidth() { return 20; };
    static int toCellLen(int pixelLen)
    {
        int l = pixelLen / cellWidth();
        l += pixelLen % cellWidth() ? 1 : 0;
        return l;
    }
    static int toPixelLen(int cellLen)
    {
        return cellLen * cellWidth();
    }
    QSize gridSize();
    QRect mapToScreenGeo(const QRect &gridGeo);
    QRect mapToGridGeo(const QRect &screenGeo);
    QPoint gridOffset();
    QMargins gridMargins();

signals:

public slots:
protected:
#ifdef QT_DEBUG
    void paintEvent(QPaintEvent *) override;
#endif
};

typedef QSharedPointer<Surface> SurfacePointer;

}

#endif   // SURFACE_H
