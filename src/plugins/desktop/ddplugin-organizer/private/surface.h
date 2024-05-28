// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SURFACE_H
#define SURFACE_H

#include "ddplugin_organizer_global.h"

#include <QWidget>
#include <QPropertyAnimation>

#include <DBlurEffectWidget>

#include <functional>

namespace ddplugin_organizer {

class ItemIndicator : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit ItemIndicator(QWidget *parent);
};

struct AnimateParams
{
    QObject *target { nullptr };
    QByteArray property;
    int duration;
    QEasingCurve curve;
    QVariant begin;
    QVariant end;
    QPropertyAnimation::KeyValues keyVals;
    std::function<void()> onFinished { nullptr };
};

class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = nullptr);
    static bool animationEnabled();
    static void animate(const AnimateParams &param);
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

    void setPositionIndicatorRect(const QRect &r);
    void activatePosIndicator(const QRect &r);
    void deactivatePosIndicator();

signals:

public slots:
protected:
    void paintEvent(QPaintEvent *) override;

private:
    QRect indicatorRect;
    ItemIndicator *indicator { nullptr };
};

typedef QSharedPointer<Surface> SurfacePointer;

}

#endif   // SURFACE_H
