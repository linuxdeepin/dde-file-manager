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

#include "screenobject.h"
#include "dbus/dbusdock.h"


#include <qpa/qplatformscreen.h>
#include <QGuiApplication>
#include <qdebug.h>

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

ScreenObject::ScreenObject(QScreen *sc, QObject *parent)
  : AbstractScreen(parent)
  ,m_screen(sc)
{
    init();
}

ScreenObject::~ScreenObject()
{

}

QString ScreenObject::name() const
{
    return m_screen->name();
}

QRect ScreenObject::geometry() const
{
    return m_screen->geometry();
}

QRect ScreenObject::availableGeometry() const
{
    //!QScreen::availableGeometry在刚启动时返回的值是错的，需要拖到下dock区才能正确显示
    //return m_screen->availableGeometry();
    //end

    QRect ret = geometry(); //已经缩放过

    int dockHideMode = DockInfoIns->hideMode();
    if ( 1 == dockHideMode) {//隐藏
        qInfo() << "dock is Hidden";
        return ret;
    }

    DockRect dockrectI = DockInfoIns->frontendWindowRect(); //原始dock大小
    QRect dockrect = dealRectRatio(dockrectI.operator QRect());  //缩放处理

#ifndef UNUSED_SMARTDOCK
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    //bug 52241
    QRect t_rect = handleGeometry();

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
            ret.setWidth(static_cast<int>(w / ratio)); //原始大小计算的宽，需缩放处理
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
            ret.setHeight(static_cast<int>(h / ratio)); //原始大小计算的高，需缩放处理
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

QRect ScreenObject::handleGeometry() const
{
    return m_screen->handle()->geometry();
}

QScreen *ScreenObject::screen() const
{
    return m_screen;
}

void ScreenObject::init()
{
    connect(m_screen,SIGNAL(geometryChanged(const QRect &)),this,SIGNAL(sigGeometryChanged(const QRect &)));
    connect(m_screen,SIGNAL(availableGeometryChanged(const QRect &)),this,SIGNAL(sigAvailableGeometryChanged(const QRect &)));
}
