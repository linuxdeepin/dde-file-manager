/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "screenproxyqt.h"
#include "screenqt.h"

#include "dbus-private/dbusdock.h"
#include "dbus-private/dbusdisplay.h"

#include <QDebug>
#include <QScreen>
#include <QApplication>

DFMBASE_USE_NAMESPACE

DSB_D_BEGIN_NAMESPACE

static bool validPrimaryChanged(const ScreenProxyQt *proxy)
{
    /*!
     * fix bug65195、bug88250 处理只有一个屏幕时，主屏发生改变的情况
     * 拔掉VGA连接线后不会触发该槽函数，通过dbus获取到的主屏名字仍然是VGA，但是Qt会创建虚拟屏幕，名字会是 :0.0 等。
     * 插上HDMI连接线会触发该槽函数，通过dbus获取到主屏的名字是HDMI，通过Qt获取到的也是HDMI，
     * 说明Qt已经从虚拟屏幕转换为真实屏幕，再配合屏幕数量只有1个，来发出屏幕改变事件。
    */
    static int times = 0;
    if (Q_LIKELY(qApp->screens().count() == 1)) {
        // 偶现，通过Qt获取到的屏幕名称可能是虚拟屏幕名称:0.0，需要延迟几百毫秒后才会获取到正确的名称（例如HDMI）
        // 规避方案：延迟100毫秒重复获取，超过10秒则放弃获取
        if (Q_UNLIKELY(QString(":0.0") == qApp->primaryScreen()->name())) {
            qWarning() << " The screen name obtained by Qt is :0.0, which is re obtained after a delay of 100 milliseconds."
                          "Current times:" << times;
            times++;
            if (Q_LIKELY(times < 100)) {
                QTimer::singleShot(100, proxy, &ScreenProxyQt::onPrimaryChanged);
            } else {
                qCritical() << "Can not get the correct primary name.Current primary name is " << qApp->primaryScreen()->name();
                times = 0;
            }
            return false;
        } else {
            qInfo() << "Primary screen changed, the screen name obtained by Qt is " << qApp->primaryScreen()->name() <<".Current times:" << times;
            times = 0;
            return true;
        }
    } else {
        // 多屏由其余正常逻辑处理，不在本特殊处理范围内
        times = 0;
        return false;
    }
}

ScreenProxyQt::ScreenProxyQt(QObject *parent)
    : AbstractScreenProxy(parent)
{

}

ScreenPointer ScreenProxyQt::primaryScreen()
{
    QScreen *primary = qApp->primaryScreen();
    ScreenPointer ret = screenMap.value(primary);
    return ret;
}

QVector<ScreenPointer> ScreenProxyQt::screens() const
{
    QVector<ScreenPointer> order;
    for (QScreen *sc : qApp->screens()) {
        if (screenMap.contains(sc)) {
            if (sc->geometry().size() == QSize(0, 0))
                qCritical() << "screen error. does it is closed?";
            order.append(screenMap.value(sc));
        }
    }
    return order;
}

QVector<ScreenPointer> ScreenProxyQt::logicScreens() const
{
    QVector<ScreenPointer> order;
    auto screens = qApp->screens();

    //调整主屏幕到第一
    QScreen *primary = qApp->primaryScreen();
    screens.removeOne(primary);
    screens.push_front(primary);

    for (QScreen *sc : screens) {
        if (screenMap.contains(sc))
            order.append(screenMap.value(sc));
    }
    return order;
}

ScreenPointer ScreenProxyQt::screen(const QString &name) const
{
    ScreenPointer ret;
    auto screens = screenMap.values();
    auto iter = std::find_if(screens.begin(), screens.end(), [name](const ScreenPointer & sp) {
        return sp->name() == name;
    });

    if (iter != screens.end()) {
        ret = *iter;
    }

    return ret;
}

qreal ScreenProxyQt::devicePixelRatio() const
{
    return qApp->primaryScreen()->devicePixelRatio();
}

DisplayMode ScreenProxyQt::displayMode() const
{
    QVector<ScreenPointer> allScreen = screens();
    if (allScreen.isEmpty())
        return DisplayMode::Custom;

    if (allScreen.size() == 1) {
        return DisplayMode::Showonly;
    } else {
        //存在两个屏幕坐标不一样则视为扩展，只有所有屏幕坐标相等发生重叠时才视为复制
        const ScreenPointer &screen = allScreen.at(0);
        for (int i = 1; i < allScreen.size(); ++i) {
            const ScreenPointer &screen2 = allScreen.at(i);
            if (screen->geometry().topLeft() != screen2->geometry().topLeft()) {
                return DisplayMode::Extend;
            }
        }

        //所有屏幕的都重叠，则视为复制
        return DisplayMode::Duplicate;
    }
}

void ScreenProxyQt::reset()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    if (display) {
        delete display;
        display = nullptr;
    }

    display = new DBusDisplay(this);

    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenProxyQt::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenProxyQt::onScreenRemoved);

    /*!
     * fix bug65195 当Qt提供了屏幕名字改变信号后，应该删除该逻辑，改为监听该信号来同步屏幕名称
     * 根因：当只存在一个屏幕时，拔插连接线（VGA和HDMI）时，不会收到Qt的屏幕添加删除信号（Qt会创建虚拟屏幕），桌面不会进行重建。
     * 但是屏幕对象返回的名字已经发生了变化，而Qt暂时没有提供屏幕名字改变的信号，导致程序中通过屏幕名字判断的逻辑都会出现问题。
     * 该bug就是由于画布中保存的名字没有更新，调用显示壁纸屏保设置界面后，找不到对应的屏幕，从而导致位置校正错误。
    */
    connect(display, &DBusDisplay::PrimaryChanged, this, &ScreenProxyQt::onPrimaryChanged);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this]() {
        this->appendEvent(Screen);
    });

    //dock区处理
    connect(DockInfoIns, &DBusDock::FrontendWindowRectChanged, this, &ScreenProxyQt::onDockChanged);
    connect(DockInfoIns, &DBusDock::HideModeChanged, this, &ScreenProxyQt::onDockChanged);

    screenMap.clear();
    for (QScreen *sc : qApp->screens()) {
        ScreenPointer psc(new ScreenQt(sc));
        screenMap.insert(sc, psc);
        connectScreen(psc);
    }

    //依赖现有屏幕数据，必须在屏幕对象初始化后调用
    lastMode = displayMode();
}

void ScreenProxyQt::onPrimaryChanged()
{
    if (validPrimaryChanged(this))
        appendEvent(Screen);
}

void ScreenProxyQt::onScreenAdded(QScreen *screen)
{
    if (screen == nullptr || screenMap.contains(screen))
        return;

    ScreenPointer psc(new ScreenQt(screen));
    screenMap.insert(screen, psc);
    connectScreen(psc);

    qInfo() << "add screen:" << screen->name();
    appendEvent(Screen);
}

void ScreenProxyQt::onScreenRemoved(QScreen *screen)
{
    auto psc = screenMap.take(screen);
    if (psc.get() != nullptr) {
        disconnectScreen(psc);
        qInfo() << "del screen:" << screen->name();
        appendEvent(Screen);
    }
}

void ScreenProxyQt::onScreenGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenProxy::Geometry);
}

void ScreenProxyQt::onScreenAvailableGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenProxy::AvailableGeometry);
}

void ScreenProxyQt::onDockChanged()
{
    appendEvent(AbstractScreenProxy::AvailableGeometry);
}

void ScreenProxyQt::processEvent()
{
    DisplayMode mode = displayMode();
    qInfo() << "current mode" << mode << "lastmode" << lastMode;

    if (mode != lastMode) {
        lastMode = mode;
        events.insert(AbstractScreenProxy::Mode, 0);
    }

    //事件优先级。由上往下，背景和画布模块在处理上层的事件已经处理过下层事件的涉及的改变，因此直接忽略
    if (events.contains(AbstractScreenProxy::Mode)) {
        emit displayModeChanged();
    } else if (events.contains(AbstractScreenProxy::Screen)) {
        emit screenChanged();
    } else if (events.contains(AbstractScreenProxy::Geometry)) {
        emit screenGeometryChanged();
    } else if (events.contains(AbstractScreenProxy::AvailableGeometry)) {
        emit screenAvailableGeometryChanged();
    }
}

void ScreenProxyQt::connectScreen(ScreenPointer sp)
{
    connect(sp.get(), &AbstractScreen::geometryChanged, this,
            &ScreenProxyQt::onScreenGeometryChanged);
}

void ScreenProxyQt::disconnectScreen(ScreenPointer sp)
{
    disconnect(sp.get(), &AbstractScreen::geometryChanged, this,
               &ScreenProxyQt::onScreenGeometryChanged);
}

DSB_D_END_NAMESPACE
