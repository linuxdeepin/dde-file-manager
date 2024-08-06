// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enterdiranimationwidget.h"
#include "dfmplugin_workspace_global.h"

#include <QPainter>
#include <QPropertyAnimation>

DPWORKSPACE_USE_NAMESPACE

EnterDirAnimationWidget::EnterDirAnimationWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    init();
}

void EnterDirAnimationWidget::setAppearPixmap(const QPixmap &pm)
{
    appearPix = pm;
}

void EnterDirAnimationWidget::setDisappearPixmap(const QPixmap &pm)
{
    disappearPix = pm;
}

void EnterDirAnimationWidget::resetWidgetSize(const QSize &size)
{
    resize(size);
}

void EnterDirAnimationWidget::playAppear()
{
    appearAnim->start();
}

void EnterDirAnimationWidget::playDisappear()
{
    appearAnim->stop();
    disappearAnim->stop();

    disappearAnim->start();
}

void EnterDirAnimationWidget::stopAndHide()
{
    appearAnim->stop();
    disappearAnim->stop();
    hide();

    appearPix = QPixmap();
    disappearPix = QPixmap();
}

void EnterDirAnimationWidget::setAppearProcess(double value)
{
    if (qFuzzyCompare(appearProcess, value))
        return;

    appearProcess = value;
}

void EnterDirAnimationWidget::setDisappearProcess(double value)
{
    if (qFuzzyCompare(disappearProcess, value))
        return;

    disappearProcess = value;
}

void EnterDirAnimationWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (disappearAnim && disappearAnim->state() == QPropertyAnimation::Running) {
        QPixmap pix = disappearPix.scaled(disappearPix.size() * (0.78 + (0.22 * disappearProcess)));
        QRect rect(0, 0, pix.width(), pix.height());
        rect.moveCenter(this->rect().center());

        painter.save();
        painter.setOpacity(disappearProcess);
        painter.drawPixmap(rect, pix);
        painter.restore();
    }

    if (appearAnim && appearAnim->state() == QPropertyAnimation::Running) {
        QPixmap pix = appearPix.scaled(appearPix.size() * (0.78 + (0.22 * appearProcess)));
        QRect rect(0, 0, pix.width(), pix.height());
        rect.moveCenter(this->rect().center());

        painter.save();
        painter.setOpacity(appearProcess);
        painter.drawPixmap(rect, pix);
        painter.restore();
    }

    QWidget::paintEvent(event);
}

void EnterDirAnimationWidget::onProcessChanged()
{
    this->update();
}

void EnterDirAnimationWidget::init()
{
    appearAnim = new QPropertyAnimation(this, "appearProcess", this);
    appearAnim->setDuration(kViewAnimationDuration);
    appearAnim->setEasingCurve(QEasingCurve::OutExpo);
    appearAnim->setStartValue(0.0);
    appearAnim->setEndValue(1.0);

    disappearAnim = new QPropertyAnimation(this, "disappearProcess", this);
    disappearAnim->setDuration(kViewAnimationDuration);
    disappearAnim->setEasingCurve(QEasingCurve::OutExpo);
    disappearAnim->setStartValue(1.0);
    disappearAnim->setEndValue(0.0);

    connect(appearAnim, &QPropertyAnimation::valueChanged,
            this, &EnterDirAnimationWidget::onProcessChanged);
    connect(disappearAnim, &QPropertyAnimation::valueChanged,
            this, &EnterDirAnimationWidget::onProcessChanged);

    connect(appearAnim, &QPropertyAnimation::finished, this, [=] {
        this->hide();
        appearPix = QPixmap();
        disappearPix = QPixmap();
    });
}
