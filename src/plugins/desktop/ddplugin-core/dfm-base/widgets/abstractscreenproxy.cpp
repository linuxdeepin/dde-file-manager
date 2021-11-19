/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "abstractscreenproxy.h"

#include <QTimer>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE
/*!
 * \brief The AbstractScreenProxy class
 * 屏幕代理创建类，可实现但不限于平台xcb与wayland
 * 此类为扩展类型，接口最终会在ScreenService中集成并使用
 */

/*!
 * \brief AbstractScreenProxy::AbstractScreenProxy
 * \param parent
 */
AbstractScreenProxy::AbstractScreenProxy(QObject *parent)
    : QObject(parent)
{
    eventShot = new QTimer(this);
    eventShot->setSingleShot(true);
    QObject::connect(eventShot, &QTimer::timeout, this, [=]() {
        processEvent();
        events.clear();
    });
}

DisplayMode AbstractScreenProxy::lastChangedMode() const
{
    return lastMode;
}

void AbstractScreenProxy::appendEvent(AbstractScreenProxy::Event e)
{
    qDebug() << "append event" << e << "current size" << (events.size() + 1);
    // 收集短时间内爆发出的事件，合并处理，优化响应速度
    events.insert(e, 0);

    eventShot->stop();
    eventShot->start(100);
}

DFMBASE_END_NAMESPACE
