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

#include "screenobjectwayland.h"
#include "dbus/dbusmonitor.h"
#include "dbus/dbusdock.h"

#include <QGuiApplication>
#include <QScreen>

static QRect dealRectRatio(QRect orgRect)
{
    //处理缩放，先不考虑高分屏的特殊处理
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    if (ratio != 1.0)
        orgRect = QRect(orgRect.x(), orgRect.y()
                        , static_cast<int>(orgRect.width() / ratio)
                        , static_cast<int>(orgRect.height() / ratio));
    return orgRect;
}

ScreenObjectWayland::ScreenObjectWayland(DBusMonitor *monitor,QObject *parent)
    : AbstractScreen(parent)
    , m_monitor(monitor)
{
    init();
}

ScreenObjectWayland::~ScreenObjectWayland()
{
    if (m_monitor){
        m_monitor->deleteLater();
        m_monitor = nullptr;
    }
}

QString ScreenObjectWayland::name() const
{
    return  m_monitor->name();
}

QRect ScreenObjectWayland::geometry() const
{
    QRect orgRect = m_monitor->rect();
    orgRect = dealRectRatio(orgRect);
    return orgRect;
}

QRect ScreenObjectWayland::availableGeometry() const
{
    QRect ret = geometry(); //已经缩放过

    int dockHideMode = DockInfoIns->hideMode();
    if ( 1 == dockHideMode){ //隐藏
        qInfo() << "dock is Hidden";
        return ret;
    }

    //DockGeoIns->getGeometry(); //经过缩放处理后的docks,有问题
    DockRect dockrectI = DockInfoIns->frontendWindowRect(); //原始dock大小
    QRect dockrect = dealRectRatio(dockrectI.operator QRect());  //缩放处理

#ifndef UNUSED_SMARTDOCK
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    //fix bug52241
    //当缩放比例为小数时，获得的缩放rect会向下整失去精度，缩放推算原始大小修改为直接获取
    QRect t_rect = handleGeometry(); //原始geometry大小

    if (!t_rect.contains(dockrectI)) { //使用原始大小判断的dock区所在的屏幕
        qDebug() << "screen:" << name() << "  handleGeometry:" << t_rect << "    dockrectI:" << dockrectI;
        return ret;
    }
#endif

    qDebug() << "frontendWindowRect: dockrectI " << QRect(dockrectI);
    qDebug() << "dealRectRatio dockrect " << dockrect;
    qDebug() << "ScreenObject ret " << ret << name();
   switch (DockInfoIns->position()) {
   case 0: //上
       ret.setY(dockrect.bottom());
       qDebug() << "dock on top, availableGeometry" << ret;
       break;
   case 1: //右
   {
       int w = dockrect.left() - ret.left();
       if (w >= 0)
           ret.setWidth(static_cast<int>(w / ratio));
       else {
           qCritical() << "dockrect.left() - ret.left() is invaild" << w;
       }
       qDebug() << "dock on right,availableGeometry" << ret;
   }
       break;
   case 2: //下
   {
       int h = dockrect.top() - ret.top();
       if (h >= 0)
           ret.setHeight(static_cast<int>(h / ratio));
       else {
           qCritical() << "dockrect.top() - ret.top() is invaild" << h;
       }
       qDebug() << "dock on bottom,availableGeometry" << ret;
       break;
   }
   case 3: //左
       ret.setX(dockrect.right());
       qDebug() << "dock on left,availableGeometry" << ret;
       break;
   default:
       qCritical() << "dock postion error!" << "and  handleGeometry:" << t_rect << "    dockrectI:" << dockrectI;
       break;
   }
    return ret;
}

QRect ScreenObjectWayland::handleGeometry() const
{
    return m_monitor->rect();
}

QString ScreenObjectWayland::path() const
{
    return  m_monitor->path();
}

bool ScreenObjectWayland::enabled() const
{
    return m_monitor->enabled();
}

void ScreenObjectWayland::init()
{
    connect(m_monitor,&DBusMonitor::monitorRectChanged, this, [this](){
        emit sigGeometryChanged(geometry());
    });
}
