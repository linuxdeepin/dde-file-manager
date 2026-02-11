// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enterdiranimationwidget.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QPainter>
#include <QPropertyAnimation>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

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
    painter.setRenderHint(QPainter::Antialiasing);
    if (disappearAnim && disappearAnim->state() == QPropertyAnimation::Running)
        paintPix(&painter, disappearPix, disappearProcess);

    if (appearAnim && appearAnim->state() == QPropertyAnimation::Running)
        paintPix(&painter, appearPix, appearProcess);

    QWidget::paintEvent(event);
}

void EnterDirAnimationWidget::onProcessChanged()
{
    this->update();
}

void EnterDirAnimationWidget::init()
{
    configScale = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterScale, 0.8).toDouble();
    configOpacity = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterOpacity, 0.0).toDouble();

    int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterDuration, 366).toInt();
    auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterCurve).toInt());

    appearAnim = new QPropertyAnimation(this, "appearProcess", this);
    appearAnim->setDuration(duration);
    appearAnim->setEasingCurve(curve);
    appearAnim->setStartValue(0.0);
    appearAnim->setEndValue(1.0);

    disappearAnim = new QPropertyAnimation(this, "disappearProcess", this);
    disappearAnim->setDuration(duration);
    disappearAnim->setEasingCurve(curve);
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

void EnterDirAnimationWidget::paintPix(QPainter *painter, const QPixmap &pix, double process)
{
    QSize paintSize = rect().size() * (configScale + ((1.0 - configScale) * process));
    QRect paintRect(0, 0, paintSize.width(), paintSize.height());
    paintRect.moveCenter(rect().center());

    painter->save();
    painter->setOpacity(configOpacity + ((1.0 - configOpacity) * process));
    painter->drawPixmap(paintRect, pix);
    painter->restore();
}
