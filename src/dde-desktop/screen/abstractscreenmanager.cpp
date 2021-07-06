/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
