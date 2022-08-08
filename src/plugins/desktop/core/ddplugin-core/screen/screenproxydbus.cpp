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
#include "screenproxydbus.h"
#include "screendbus.h"
#include "dbus-private/dbusdisplay.h"
#include "dbus-private/dbusmonitor.h"
#include "dbus-private/dbusdock.h"

#include <QApplication>
#include <QScreen>
#include <QDebug>

DFMBASE_USE_NAMESPACE

DDPCORE_USE_NAMESPACE

#define SCREENOBJECT(screen) dynamic_cast<ScreenDBus*>(screen)

ScreenProxyDBus::ScreenProxyDBus(QObject *parent)
    : AbstractScreenProxy(parent)
{

}

ScreenPointer ScreenProxyDBus::primaryScreen()
{
    QString primaryName = display->primary();
    if (primaryName.isEmpty())
        qCritical() << "get primary name failed";

    ScreenPointer ret;
    for (ScreenPointer sp : screenMap.values()) {
        if (sp->name() == primaryName) {
            ret = sp;
            break;
        }
    }
    if (ret.isNull())
        qWarning() << "get primary from dbus:" <<primaryName << ".save monitors:" << screenMap.keys();
    return ret;
}

QVector<ScreenPointer> ScreenProxyDBus::screens() const
{
    QVector<ScreenPointer> order;
    for (const QDBusObjectPath &path : display->monitors()) {
        if (screenMap.contains(path.path())) {
            ScreenPointer sp = screenMap.value(path.path());
            ScreenDBus *screen = SCREENOBJECT(sp.data());
            if (screen) {
                if (screen->enabled())
                    order.append(sp);
            } else
                order.append(sp);
        } else {
            qWarning() << "unknow monitor:" << path.path() << ".save monitors:" << screenMap.keys();
        }
    }
    return order;
}

QVector<ScreenPointer> ScreenProxyDBus::logicScreens() const
{
    QVector<ScreenPointer> order;
    QString primaryName = display->primary();
    if (primaryName.isEmpty())
        qCritical() << "get primary name failed";

    //调整主屏幕到第一
    for (const QDBusObjectPath &path : display->monitors()) {
        if (path.path().isEmpty()) {
            qWarning() << "monitor: QDBusObjectPath is empty";
            continue;
        }

        if (screenMap.contains(path.path())) {
            ScreenPointer sp = screenMap.value(path.path());
            if (sp == nullptr) {
                qCritical() << "get scrreen failed path" << path.path();
                continue;
            }
            if (sp->name() == primaryName) {
                order.push_front(sp);
            } else {
                ScreenDBus *screen = SCREENOBJECT(sp.data());
                if (screen) {
                    if (screen->enabled())
                        order.push_back(sp);
                } else
                    order.push_back(sp);
            }
        }
    }
    return order;
}

ScreenPointer ScreenProxyDBus::screen(const QString &name) const
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

qreal ScreenProxyDBus::devicePixelRatio() const
{
    //dbus获取的缩放不是应用值而是设置值，目前还是使用QT来获取
    return qApp->primaryScreen()->devicePixelRatio();
}

DisplayMode ScreenProxyDBus::displayMode() const
{
    auto pending = display->GetRealDisplayMode();
    pending.waitForFinished();
    if (pending.isError()) {
        qWarning() << "Display GetRealDisplayMode Error:" << pending.error().name() << pending.error().message();
        DisplayMode ret = DisplayMode(display->displayMode());
        return ret;
    } else {
        /*
        DisplayModeMirror: 1
        DisplayModeExtend: 2
        DisplayModeOnlyOne: 3
        DisplayModeUnknow: 4
        */
        int mode = pending.argumentAt(0).toInt();
        qDebug() << "GetRealDisplayMode resulet" << mode;
        if (mode > 0 && mode < 4)
            return static_cast<DisplayMode>(mode);
        else
            return kCustom;
    }
}

void ScreenProxyDBus::reset()
{
    if (display) {
        delete display;
        display = nullptr;
    }

    display = new DBusDisplay(this);
    screenMap.clear();

    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenProxyDBus::onMonitorChanged);
    connect(display, &DBusDisplay::MonitorsChanged, this, &ScreenProxyDBus::onMonitorChanged);
    connect(display, &DBusDisplay::PrimaryChanged, this, [this]() {
        this->appendEvent(kScreen);
    });

    connect(display, &DBusDisplay::DisplayModeChanged, this, &ScreenProxyDBus::onModeChanged);

    //使用PrimaryRectChanged信号作为拆分/合并信号
    connect(display, &DBusDisplay::PrimaryRectChanged, this, &ScreenProxyDBus::onModeChanged);

    lastMode = static_cast<DisplayMode>(display->GetRealDisplayMode().value());

    //dock区处理
    connect(DockInfoIns, &DBusDock::FrontendWindowRectChanged, this, &ScreenProxyDBus::onDockChanged);
    connect(DockInfoIns, &DBusDock::HideModeChanged, this, &ScreenProxyDBus::onDockChanged);

    //初始化屏幕
    for (auto objectPath : display->monitors()) {
        const QString path = objectPath.path();
        ScreenPointer sp(new ScreenDBus(new DBusMonitor(path)));
        screenMap.insert(path, sp);
        connectScreen(sp);
    }
}

void ScreenProxyDBus::processEvent()
{
    //事件优先级。由上往下，背景和画布模块在处理上层的事件已经处理过下层事件的涉及的改变，因此直接忽略
    if (events.contains(kMode)) {
        emit displayModeChanged();
    } else if (events.contains(kScreen)) {
        emit screenChanged();
    } else if (events.contains(kGeometry)) {
        emit screenGeometryChanged();
    } else if (events.contains(kAvailableGeometry)) {
        emit screenAvailableGeometryChanged();
    }
}

void ScreenProxyDBus::onMonitorChanged()
{
    QStringList monitors;
    //检查新增的屏幕
    for (auto objectPath : display->monitors()) {
        QString path = objectPath.path();
        if (path.isEmpty()) {
            qWarning() << "get monitor path is empty from display";
            continue;
        }

        //新增的
        if (!screenMap.contains(path)) {
            ScreenPointer sp(new ScreenDBus(new DBusMonitor(path)));
            screenMap.insert(path, sp);
            connectScreen(sp);
            qInfo() << "add monitor:" << path;
        }
        monitors << path;
    }
    qDebug() << "get monitors:" << monitors;

    //检查移除的屏幕
    for (const QString &path : screenMap.keys()) {
        if (!monitors.contains(path)) {
            ScreenPointer sp = screenMap.take(path);
            disconnectScreen(sp);
            qInfo() << "del monitor:" << path;
        }
    }
    qDebug() << "save monitors:" << screenMap.keys();
    appendEvent(kScreen);
}

void ScreenProxyDBus::onDockChanged()
{
    appendEvent(kAvailableGeometry);
}

void ScreenProxyDBus::onScreenGeometryChanged(const QRect &rect)
{
    appendEvent(kGeometry);

    //fix wayland下切换合并/拆分，当主屏的geometry在合并拆分前后没有改变时，不会发送PrimaryRectChanged，而在主线能发送出来。
    //这导致没法判断到显示模式改变，这里补充触发
    emit display->PrimaryRectChanged();
}

void ScreenProxyDBus::onModeChanged()
{
    DisplayMode mode = static_cast<DisplayMode>(display->GetRealDisplayMode().value());
    qInfo() << "deal display mode/primary rect changed " << mode;
    if (lastMode == mode)
        return;
    lastMode = mode;
    this->appendEvent(kMode);
}

void ScreenProxyDBus::connectScreen(ScreenPointer sp)
{
    connect(sp.get(), &AbstractScreen::geometryChanged, this,
            &ScreenProxyDBus::onScreenGeometryChanged);
}

void ScreenProxyDBus::disconnectScreen(ScreenPointer sp)
{
    disconnect(sp.get(), &AbstractScreen::geometryChanged, this,
            &ScreenProxyDBus::onScreenGeometryChanged);
}
