// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enterdiranimationwidget.h"
#include "dfmplugin_workspace_global.h"

#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

DPWORKSPACE_USE_NAMESPACE

EnterDirAnimationWidget::EnterDirAnimationWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    init();
}

void EnterDirAnimationWidget::setPixmap(const QPixmap &pix)
{
    if (!freezePixmapContainer)
        return;

    freezePixmapContainer->setPixmap(pix);
    freezePixmapContainer->resize(pix.width(), pix.height());
    blankBackground->resize(pix.width(), pix.height());
}

void EnterDirAnimationWidget::setScaleAnimParam(const QRect &start, const QRect &end)
{
    if (!scaleAnim)
        return;

    scaleAnim->setStartValue(start);
    scaleAnim->setEndValue(end);
}

void EnterDirAnimationWidget::setTrasparentAnimParam(qreal start, qreal end)
{
    if (!transparentAnim)
        return;

    transparentAnim->setStartValue(start);
    transparentAnim->setEndValue(end);
}

void EnterDirAnimationWidget::setBlankBackgroundVisiable(bool visible)
{
    if (!blankBackground)
        return;

    blankBackground->setVisible(visible);
}

void EnterDirAnimationWidget::play()
{
    if (!scaleAnim || !transparentAnim)
        return;

    // if (freezePixmapContainer->pixmap()->isNull())
    //     return;

    scaleAnim->start();
    transparentAnim->start();
}

void EnterDirAnimationWidget::playAppear()
{
    if (!scaleAnim || !transparentAnim)
        return;

    auto pix = freezePixmapContainer->pixmap();
    if (pix->isNull())
        return;

    QRect endRect = freezePixmapContainer->rect();
    qreal pixWidth = pix->width();
    qreal pixHeight = pix->height();
    QRect startRect = endRect.adjusted(pixWidth * 0.11, pixHeight * 0.11, -(pixWidth * 0.11), -(pixHeight * 0.11));
    startRect.moveCenter(endRect.center());
    scaleAnim->setStartValue(startRect);
    scaleAnim->setEndValue(endRect);

    transparentAnim->setStartValue(0.0);
    transparentAnim->setEndValue(1.0);

    freezePixmapContainer->show();
    scaleAnim->start();
    transparentAnim->start();
}

void EnterDirAnimationWidget::playDisappear()
{
    if (!scaleAnim || !transparentAnim)
        return;

    auto pix = freezePixmapContainer->pixmap();
    if (pix->isNull())
        return;

    QRect startRect = freezePixmapContainer->rect();
    qreal pixWidth = pix->width();
    qreal pixHeight = pix->height();
    QRect endRect = startRect.adjusted(pixWidth * 0.11, pixHeight * 0.11, -(pixWidth * 0.11), -(pixHeight * 0.11));
    endRect.moveCenter(startRect.center());
    scaleAnim->setStartValue(startRect);
    scaleAnim->setEndValue(endRect);

    transparentAnim->setStartValue(1.0);
    transparentAnim->setEndValue(0.0);

    freezePixmapContainer->show();
    scaleAnim->start();
    transparentAnim->start();
}

void EnterDirAnimationWidget::init()
{
    blankBackground = new QLabel(this);
    blankBackground->setAutoFillBackground(true);
    blankBackground->setVisible(false);

    freezePixmapContainer = new QLabel(this);
    freezePixmapContainer->setScaledContents(true);

    scaleAnim = new QPropertyAnimation(freezePixmapContainer, "geometry", this);
    scaleAnim->setDuration(366);
    scaleAnim->setEasingCurve(QEasingCurve::OutExpo);

    QGraphicsOpacityEffect *opaEffect = new QGraphicsOpacityEffect(freezePixmapContainer);
    opaEffect->setOpacity(1);
    freezePixmapContainer->setGraphicsEffect(opaEffect);

    transparentAnim = new QPropertyAnimation(opaEffect, "opacity", this);
    transparentAnim->setDuration(366);
    transparentAnim->setEasingCurve(QEasingCurve::OutExpo);

    connect(transparentAnim, &QPropertyAnimation::finished, this, [=] {
        freezePixmapContainer->hide();
        this->hide();
    });
}
