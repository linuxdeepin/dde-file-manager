// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>

#include <QTimer>
#include <QDebug>

namespace dfmbase {
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
    qCDebug(logDFMBase) << "append event" << e << "current size" << (events.size() + 1);
    // 收集短时间内爆发出的事件，合并处理，优化响应速度
    events.insert(e, 0);

    eventShot->stop();
    eventShot->start(100);
}

}
