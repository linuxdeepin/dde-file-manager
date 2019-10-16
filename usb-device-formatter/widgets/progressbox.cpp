/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "progressbox.h"
#include <QDebug>

ProgressBox::ProgressBox(QWidget *parent) :
    DWaterProgress(parent),
    m_taskTimer(new QTimer(this)),
    m_updateTimer(new QTimer(this)),
    m_taskAni(new QVariantAnimation(this))
{
    m_taskTimer->setInterval(100);
    m_taskTimer->setSingleShot(true);

    m_updateTimer->setInterval(66);
    m_updateTimer->start();

    initUI();
    initConnections();
}

void ProgressBox::initConnections()
{
    connect(m_taskTimer, &QTimer::timeout, this, &ProgressBox::taskTimeOut);
    connect(m_updateTimer, &QTimer::timeout, this, &ProgressBox::updateAnimation);
}

void ProgressBox::initUI()
{
    setFixedSize(128, 128);
    start();
}

void ProgressBox::updateAnimation()
{
    update();
}

void ProgressBox::taskTimeOut()
{
    m_taskAni->stop();
    m_taskAni->setDuration(10000); //10 seconds
    m_taskAni->setStartValue(0);
    m_taskAni->setEndValue(99);
    m_taskAni->setEasingCurve(QEasingCurve::OutCirc);
    connect(m_taskAni, &QVariantAnimation::valueChanged, this, [=] (const QVariant& val) {
        setValue(val.toInt());
    });
    m_taskAni->start();
}

void ProgressBox::startTask()
{
    m_taskTimer->start();
}

void ProgressBox::finishedTask(const bool result)
{
    m_taskAni->stop();
    QVariantAnimation* ani = new QVariantAnimation(this);
    ani->setDuration(300);
    ani->setStartValue(value());
    ani->setEndValue(100);
    connect(ani, &QVariantAnimation::valueChanged, this, [=] (const QVariant& val){
        setValue(val.toInt());
    });
    connect(ani, &QVariantAnimation::finished, [=]{
        ani->deleteLater();
        setValue(0);
        emit finished(result);
    });
    ani->start();
}
