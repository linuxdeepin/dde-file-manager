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

#include "screenmanager.h"
#include "screenobject.h"
#include "abstractscreenmanager_p.h"
#include "dbus/dbusdisplay.h"
#include "dbus/dbusdock.h"


#include <QGuiApplication>

#define SCREENOBJECT(screen) dynamic_cast<ScreenObject*>(screen)


static bool validPrimaryChanged(const ScreenManager *screenManager)
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
                QTimer::singleShot(100, screenManager, &ScreenManager::onPrimaryChanged);
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


ScreenManager::ScreenManager(QObject *parent)
    : AbstractScreenManager(parent)
{
    m_display = new DBusDisplay(this);
    init();
}

ScreenManager::~ScreenManager()
{

}

void ScreenManager::onScreenAdded(QScreen *screen)
{
    if (screen == nullptr || m_screens.contains(screen))
        return;

    ScreenObjectPointer psc(new ScreenObject(screen));
    m_screens.insert(screen, psc);
    connectScreen(psc);

    qInfo() << "add screen:" << screen->name();
    //emit sigScreenChanged();
    appendEvent(Screen);
}

void ScreenManager::onScreenRemoved(QScreen *screen)
{
    auto psc = m_screens.take(screen);
    if (psc.get() != nullptr) {
        disconnectScreen(psc);
        qInfo() << "del screen:" << screen->name();
        //emit sigScreenChanged();
        appendEvent(Screen);
    }
}

void ScreenManager::onPrimaryChanged()
{
    if (validPrimaryChanged(this))
        appendEvent(Screen);
}

void ScreenManager::onScreenGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenManager::Geometry);
}

void ScreenManager::onScreenAvailableGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenManager::AvailableGeometry);
}

void ScreenManager::onDockChanged()
{
//    int dockHideMode = DockInfoIns->hideMode();
//    if (3 != dockHideMode) //只处理智能隐藏
//        return ;
#ifdef UNUSED_SMARTDOCK
    auto primary = primaryScreen();
    if (primary == nullptr) {
        qCritical() << "get primary screen failed";
        return;
    } else {
        emit sigScreenAvailableGeometryChanged(primary, primary->availableGeometry());
    }
#else
    //新增动态dock区功能，dock区不再只是在主屏幕,随鼠标移动
    appendEvent(AbstractScreenManager::AvailableGeometry);
#endif
}

void ScreenManager::init()
{
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManager::onScreenRemoved);

    /*!
     * fix bug65195 临时方案。当Qt提供了屏幕名字改变信号后，应该删除该逻辑，改为监听该信号来同步屏幕名称
     * 根因：当只存在一个屏幕时，拔插连接线（VGA和HDMI）时，不会收到Qt的屏幕添加删除信号（Qt会创建虚拟屏幕），桌面不会进行重建。
     * 但是屏幕对象返回的名字已经发生了变化，而Qt暂时没有提供屏幕名字改变的信号，导致程序中通过屏幕名字判断的逻辑都会出现问题。
     * 该bug就是由于画布中保存的名字没有更新，调用显示壁纸屏保设置界面后，找不到对应的屏幕，从而导致位置校正错误。
    */
    connect(m_display, &DBusDisplay::PrimaryChanged, this, &ScreenManager::onPrimaryChanged);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this]() {
        this->appendEvent(Screen);
    });

    //dock区处理
    connect(DockInfoIns, &DBusDock::FrontendWindowRectChanged, this, &ScreenManager::onDockChanged);
    connect(DockInfoIns, &DBusDock::HideModeChanged, this, &ScreenManager::onDockChanged);
    //connect(DockInfoIns,&DBusDock::PositionChanged,this, &ScreenManager::onDockChanged); 不关心位子改变，有bug#25148，全部由区域改变触发

    m_screens.clear();
    for (QScreen *sc : qApp->screens()) {
        ScreenPointer psc(new ScreenObject(sc));
        m_screens.insert(sc, psc);
        connectScreen(psc);
    }

    //依赖现有屏幕数据，必须在屏幕对象初始化后调用
    m_lastMode = displayMode();
}

void ScreenManager::connectScreen(ScreenPointer psc)
{
    connect(psc.get(), &AbstractScreen::sigGeometryChanged, this,
            &ScreenManager::onScreenGeometryChanged);
//为了解决bug33117，
//在dock时尚模式下，去切换dock位置后，会先触发QScreen的sigAvailableGeometryChanged，此时去获取
//dock的区域不正确，继而导致桌面栅格计算不正确从而导致自定义下记录的桌面图标无法恢复而异常位置。先暂时屏蔽之。
//至于此处信号会影响什么地方暂时不可追溯（目前在x86单双屏切换dock均未发现有何影响）。

//    connect(psc.get(),&AbstractScreen::sigAvailableGeometryChanged,this,
//            &ScreenManager::onScreenAvailableGeometryChanged);
}


void ScreenManager::disconnectScreen(ScreenPointer psc)
{
    disconnect(psc.get(), &AbstractScreen::sigGeometryChanged, this,
               &ScreenManager::onScreenGeometryChanged);
    disconnect(psc.get(), &AbstractScreen::sigAvailableGeometryChanged, this,
               &ScreenManager::onScreenAvailableGeometryChanged);
}

ScreenPointer ScreenManager::primaryScreen()
{
    QScreen *primary = qApp->primaryScreen();
    ScreenPointer ret = m_screens.value(primary);
    //Q_ASSERT(ret.get() != nullptr);
    return ret;
}

QVector<ScreenPointer> ScreenManager::screens() const
{
    QVector<ScreenPointer> order;
    for (QScreen *sc : qApp->screens()) {
        if (m_screens.contains(sc)) {
            if (sc->geometry().size() == QSize(0, 0))
                qCritical() << "screen error. does it is closed?";
            order.append(m_screens.value(sc));
        }
    }
    return order;
}

QVector<ScreenPointer> ScreenManager::logicScreens() const
{
    QVector<ScreenPointer> order;
    auto screens = qApp->screens();

    //调整主屏幕到第一
    QScreen *primary = qApp->primaryScreen();
    screens.removeOne(primary);
    screens.push_front(primary);

    for (QScreen *sc : screens) {
        if (m_screens.contains(sc))
            order.append(m_screens.value(sc));
    }
    return order;
}

ScreenPointer ScreenManager::screen(const QString &name) const
{
    ScreenPointer ret;
    auto screens = m_screens.values();
    auto iter = std::find_if(screens.begin(), screens.end(), [name](const ScreenPointer & sp) {
        return sp->name() == name;
    });

    if (iter != screens.end()) {
        ret = *iter;
    }

    return ret;
}

qreal ScreenManager::devicePixelRatio() const
{
    return qApp->primaryScreen()->devicePixelRatio();
}

AbstractScreenManager::DisplayMode ScreenManager::displayMode() const
{
    QVector<ScreenPointer> allScreen = screens();
    if (allScreen.isEmpty())
        return AbstractScreenManager::Custom;

    if (allScreen.size() == 1) {
        return AbstractScreenManager::Showonly;
    } else {
        //存在两个屏幕坐标不一样则视为扩展，只有所有屏幕坐标相等发生重叠时才视为复制
        const ScreenPointer &screen = allScreen.at(0);
        for (int i = 1; i < allScreen.size(); ++i) {
            const ScreenPointer &screen2 = allScreen.at(i);
            if (screen->geometry().topLeft() != screen2->geometry().topLeft()) {
                return AbstractScreenManager::Extend;
            }
        }

        //所有屏幕的都重叠，则视为复制
        return AbstractScreenManager::Duplicate;
    }
}

AbstractScreenManager::DisplayMode ScreenManager::lastChangedMode() const
{
    return static_cast<AbstractScreenManager::DisplayMode>(m_lastMode);
}

void ScreenManager::reset()
{
    if (m_display) {
        delete m_display;
        m_display = nullptr;
    }

    m_display = new DBusDisplay(this);
    init();
}

void ScreenManager::processEvent()
{
    //x11下通过屏幕数据计算显示模式
    //因此需要在每次屏幕变化时计算当前的显示模式
    int mode = displayMode();
    qInfo() << "current mode" << mode << "lastmode" << m_lastMode;

    if (mode != m_lastMode) {
        m_lastMode = mode;
        d->m_events.insert(AbstractScreenManager::Mode, 0);
    }

    //事件优先级。由上往下，背景和画布模块在处理上层的事件已经处理过下层事件的涉及的改变，因此直接忽略
    if (d->m_events.contains(AbstractScreenManager::Mode)) {
        emit sigDisplayModeChanged();
    }
    else if (d->m_events.contains(AbstractScreenManager::Screen)) {
        emit sigScreenChanged();
    }
    else if (d->m_events.contains(AbstractScreenManager::Geometry)) {
        emit sigScreenGeometryChanged();
    }
    else if (d->m_events.contains(AbstractScreenManager::AvailableGeometry)) {
        emit sigScreenAvailableGeometryChanged();
    }
}
