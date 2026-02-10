// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenproxyqt.h"
#include "screenqt.h"

#include "dbus-private/dbushelper.h"

#include <dfm-base/utils/windowutils.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QScreen>
#include <QApplication>
#include <QStringList>

Q_DECLARE_METATYPE(QStringList *)

DFMBASE_USE_NAMESPACE

DDPCORE_USE_NAMESPACE

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
            fmWarning() << " The screen name obtained by Qt is :0.0, which is re obtained after a delay of 100 milliseconds."
                           "Current times:"
                        << times;
            times++;
            if (Q_LIKELY(times < 100)) {
                QTimer::singleShot(100, proxy, &ScreenProxyQt::onPrimaryChanged);
            } else {
                fmCritical() << "Can not get the correct primary name.Current primary name is " << qApp->primaryScreen()->name();
                times = 0;
            }
            return false;
        } else {
            fmInfo() << "Primary screen changed, the screen name obtained by Qt is " << qApp->primaryScreen()->name()
                     << ".Current times:" << times;
            //! When using dual-screen mode with only one screen, if the screen is switched from Screen 1 to Screen 2
            //! in the Control Center, Qt does not emit a related screen change signal.
            //! Therefore, the primary screen changed signal is handled through the dde display service.
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

QList<ScreenPointer> ScreenProxyQt::screens() const
{
    QList<ScreenPointer> order;
    for (QScreen *sc : qApp->screens()) {
        if (screenMap.contains(sc)) {
            if (sc->name().isEmpty() || sc->geometry().size() == QSize(0, 0)) {
                fmCritical() << "Invalid screen detected - name:" << sc->name() << "geometry:" << sc->geometry();
                continue;
            }
            order.append(screenMap.value(sc));
        } else {
            fmDebug() << "Screen not found in map:" << sc->name();
        }
    }
    return order;
}

QList<ScreenPointer> ScreenProxyQt::logicScreens() const
{
    QList<ScreenPointer> order;
    auto allScreen = qApp->screens();

    // 调整主屏幕到第一
    QScreen *primary = qApp->primaryScreen();
    allScreen.removeOne(primary);
    allScreen.push_front(primary);

    fmDebug() << "Building logic screens list with primary screen first:" << (primary ? primary->name() : "null");

    for (QScreen *sc : allScreen) {
        if (screenMap.contains(sc)) {
            if (sc->name().isEmpty() || sc->geometry().size() == QSize(0, 0)) {
                fmCritical() << "Invalid screen in logic list - name:" << sc->name() << "geometry:" << sc->geometry();
                continue;
            }
            order.append(screenMap.value(sc));
        } else {
            fmDebug() << "Screen not found in map for logic screens:" << sc->name();
        }
    }
    fmDebug() << "Returning" << order.size() << "logic screens";
    return order;
}

ScreenPointer ScreenProxyQt::screen(const QString &name) const
{
    ScreenPointer ret;
    auto allScreen = screenMap.values();
    auto iter = std::find_if(allScreen.begin(), allScreen.end(), [name](const ScreenPointer &sp) {
        return sp->name() == name;
    });

    if (iter != allScreen.end()) {
        ret = *iter;
        fmDebug() << "Screen found by name:" << name;
    } else {
        fmDebug() << "Screen not found by name:" << name;
    }

    return ret;
}

qreal ScreenProxyQt::devicePixelRatio() const
{
    return qApp->primaryScreen()->devicePixelRatio();
}

DisplayMode ScreenProxyQt::displayMode() const
{
#ifdef COMPILE_ON_V2X
    if (DFMBASE_NAMESPACE::WindowUtils::isWayLand()) {
        fmDebug() << "Wayland environment detected, using show-only mode";
        return DisplayMode::kShowonly;
    }
#endif

    QList<ScreenPointer> allScreen = screens();
    if (allScreen.isEmpty()) {
        fmWarning() << "No screens available, using custom mode";
        return DisplayMode::kCustom;
    }

    if (allScreen.size() == 1) {
        fmDebug() << "Single screen detected, using show-only mode";
        return DisplayMode::kShowonly;
    } else {
        // 存在两个屏幕坐标不一样则视为扩展，只有所有屏幕坐标相等发生重叠时才视为复制
        const ScreenPointer &screen1 = allScreen.at(0);
        for (int i = 1; i < allScreen.size(); ++i) {
            const ScreenPointer &screen2 = allScreen.at(i);
            if (screen1->geometry().topLeft() != screen2->geometry().topLeft()) {
                fmDebug() << "Multiple screens with different positions detected, using extend mode";
                return DisplayMode::kExtend;
            }
        }

        // 所有屏幕的都重叠，则视为复制
        fmDebug() << "Multiple screens with same position detected, using duplicate mode";
        return DisplayMode::kDuplicate;
    }
}

void ScreenProxyQt::reset()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenProxyQt::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenProxyQt::onScreenRemoved);

    /*!
     * fix bug65195 当Qt提供了屏幕名字改变信号后，应该删除该逻辑，改为监听该信号来同步屏幕名称
     * 根因：当只存在一个屏幕时，拔插连接线（VGA和HDMI）时，不会收到Qt的屏幕添加删除信号（Qt会创建虚拟屏幕），桌面不会进行重建。
     * 但是屏幕对象返回的名字已经发生了变化，而Qt暂时没有提供屏幕名字改变的信号，导致程序中通过屏幕名字判断的逻辑都会出现问题。
     * 该bug就是由于画布中保存的名字没有更新，调用显示壁纸屏保设置界面后，找不到对应的屏幕，从而导致位置校正错误。
     */
    connect(DisplayInfoIns, &DBusDisplay::PrimaryChanged, this, &ScreenProxyQt::onPrimaryChanged);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this]() {
        this->appendEvent(kScreen);
    });

    // dock区处理
    connect(DockInfoIns, &DBusDock::FrontendWindowRectChanged, this, &ScreenProxyQt::onDockChanged);
    connect(DockInfoIns, &DBusDock::HideModeChanged, this, &ScreenProxyQt::onDockChanged);

    screenMap.clear();
    fmInfo() << "Initializing" << qApp->screens().size() << "screens";
    for (QScreen *sc : qApp->screens()) {
        ScreenPointer psc(new ScreenQt(sc));
        screenMap.insert(sc, psc);
        connectScreen(psc);
        fmDebug() << "Screen initialized:" << sc->name() << "geometry:" << sc->geometry();
    }

    // 依赖现有屏幕数据，必须在屏幕对象初始化后调用
    lastMode = displayMode();
}

void ScreenProxyQt::onPrimaryChanged()
{
    if (validPrimaryChanged(this))
        appendEvent(kScreen);
}

void ScreenProxyQt::onScreenAdded(QScreen *screen)
{
    if (screen == nullptr) {
        fmWarning() << "Null screen pointer in onScreenAdded";
        return;
    }

    if (screenMap.contains(screen)) {
        fmDebug() << "Screen already exists in map:" << screen->name();
        return;
    }

    fmInfo() << "Adding new screen:" << screen->name() << "geometry:" << screen->geometry();
    ScreenPointer psc(new ScreenQt(screen));
    screenMap.insert(screen, psc);
    connectScreen(psc);

    fmInfo() << "Screen added successfully:" << screen->name() << "total screens:" << screenMap.size();
    appendEvent(kScreen);
}

void ScreenProxyQt::onScreenRemoved(QScreen *screen)
{
    auto psc = screenMap.take(screen);
    if (psc.get() != nullptr) {
        disconnectScreen(psc);
        fmInfo() << "Screen removed:" << screen->name() << "remaining screens:" << screenMap.size();
        appendEvent(kScreen);
    } else {
        fmDebug() << "Attempted to remove non-existent screen:" << screen->name();
    }
}

void ScreenProxyQt::onScreenGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenProxy::kGeometry);
}

void ScreenProxyQt::onScreenAvailableGeometryChanged(const QRect &rect)
{
    Q_UNUSED(rect)
    appendEvent(AbstractScreenProxy::kAvailableGeometry);
}

void ScreenProxyQt::onDockChanged()
{
    appendEvent(AbstractScreenProxy::kAvailableGeometry);
}

void ScreenProxyQt::processEvent()
{
    DisplayMode mode = displayMode();
    fmDebug() << "Processing events - current mode:" << mode << "last mode:" << lastMode;

    if (mode != lastMode) {
        fmInfo() << "Display mode changed from" << lastMode << "to" << mode;
        lastMode = mode;
        events.insert(AbstractScreenProxy::kMode, 0);
    }

    // check whether primary screen changed by fake screen
    // then object point to a fake screen(:0.0) that is resotred to a real screen(like hdmi) will change its name and rect.
    // then it just emits geometry changed signal and using name of the screen(hdmi) changed from :0.0 to find window relate to :0.0
    // is invalid.
    if (!events.contains(AbstractScreenProxy::kMode) && !events.contains(AbstractScreenProxy::kScreen)) {
        if (!checkUsedScreens()) {
            fmWarning() << "Used screen check failed, adding screen event";
            events.insert(AbstractScreenProxy::kScreen, 0);
        }
    }

    // 事件优先级。由上往下，背景和画布模块在处理上层的事件已经处理过下层事件的涉及的改变，因此直接忽略
    if (events.contains(AbstractScreenProxy::kMode)) {
        emit displayModeChanged();
    } else if (events.contains(AbstractScreenProxy::kScreen)) {
        emit screenChanged();
    } else if (events.contains(AbstractScreenProxy::kGeometry)) {
        emit screenGeometryChanged();
    } else if (events.contains(AbstractScreenProxy::kAvailableGeometry)) {
        emit screenAvailableGeometryChanged();
    }
}

bool ScreenProxyQt::checkUsedScreens()
{
    QSet<QString> cur;
    for (auto sc : screenMap.keys())
        cur << sc->name();

    QStringList scs;
    dpfHookSequence->run("ddplugin_core", "hook_ScreenProxy_ScreensInUse", &scs);
    QSet<QString> inuse(scs.begin(), scs.end());
    fmDebug() << "Checking used screens - current:" << cur << "in use:" << inuse;

    bool invalid = false;
    for (const QString &sc : inuse) {
        if (cur.contains(sc))
            continue;
        invalid = true;
        fmWarning() << "Screen was lost:" << sc;
        break;
    }

    return !invalid;
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
