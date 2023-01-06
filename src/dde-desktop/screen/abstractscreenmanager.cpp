// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractscreenmanager.h"
#include "abstractscreenmanager_p.h"

#include <QDebug>

ScreenManagerPrivate::ScreenManagerPrivate(AbstractScreenManager *p): q(p)
{
    QObject::connect(&m_eventShot, &QTimer::timeout, q, [=]() {
        q->processEvent();
        m_events.clear();
    });
}

ScreenManagerPrivate::~ScreenManagerPrivate()
{
}

void ScreenManagerPrivate::readyShot(int wait)
{
    m_eventShot.stop();
    qDebug() << "shotting later " << wait;

    if (wait < 1) {
        wait = 1;
    }

    m_eventShot.setSingleShot(true);
    m_eventShot.start(wait);
}

AbstractScreenManager::AbstractScreenManager(QObject *parent)
    : QObject(parent)
    ,d(new ScreenManagerPrivate(this))
{

}

AbstractScreenManager::~AbstractScreenManager()
{
    delete d;
}

void AbstractScreenManager::appendEvent(AbstractScreenManager::Event e)
{
    qDebug() << "append event" << e << "current size" << (d->m_events.size() + 1);
    //收集短时间内爆发出的事件，合并处理，优化响应速度
    d->m_events.insert(e,0);
    d->readyShot(100);
}
