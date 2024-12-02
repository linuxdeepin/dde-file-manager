// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenqt.h"
#include "dbus-private/dbushelper.h"

#include <qpa/qplatformscreen.h>
#include <QDebug>
#include <QApplication>

namespace GlobalPrivate {
static QRect dealRectRatio(QRect orgRect)
{
    //处理缩放，先不考虑高分屏的特殊处理
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    if (ratio != 1.0)
        orgRect = QRect(orgRect.x(), orgRect.y(), static_cast<int>(orgRect.width() / ratio), static_cast<int>(orgRect.height() / ratio));
    return orgRect;
}
}   // namespace GlobalPrivate

DDPCORE_USE_NAMESPACE

ScreenQt::ScreenQt(QScreen *screen, QObject *parent)
    : DFMBASE_NAMESPACE::AbstractScreen(parent), qscreen(screen)
{
    Q_ASSERT(qscreen);
    connect(qscreen, SIGNAL(geometryChanged(const QRect &)), this, SIGNAL(geometryChanged(const QRect &)));
    connect(qscreen, SIGNAL(availableGeometryChanged(const QRect &)), this, SIGNAL(availableGeometryChanged(const QRect &)));
}

QString ScreenQt::name() const
{
    return qscreen->name();
}

QRect ScreenQt::geometry() const
{
    return qscreen->geometry();
}

QRect ScreenQt::availableGeometry() const
{
    //!QScreen::availableGeometry在刚启动时返回的值是错的，需要拖到下dock区才能正确显示
    //return m_screen->availableGeometry();
    //end

    QRect ret = geometry();   //已经缩放过

    if (!DBusHelper::isDockEnable()) {
        fmWarning() << "dde dock is not registered";
        return ret;
    }

    int dockHideMode = DockInfoIns->hideMode();
    if (1 == dockHideMode) {   //隐藏
        fmInfo() << "dock is Hidden";
        return ret;
    }

    DockRect dockrectI = DockInfoIns->frontendWindowRect();   //原始dock大小
    const QRect dockrect = GlobalPrivate::dealRectRatio(dockrectI.operator QRect());   //缩放处理

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    const QRect hRect = handleGeometry();

    if (!hRect.contains(dockrectI)) {   //使用原始大小判断的dock区所在的屏幕
        fmDebug() << "screen:" << name() << "  handleGeometry:" << hRect << "    dockrectI:" << dockrectI;
        return ret;
    }

    fmDebug() << "ScreenQt ret " << ret << name();
    const int dockPos = DockInfoIns->position();
    switch (dockPos) {
    case 0:   //上
        ret.setY(dockrect.bottom());
        fmDebug() << "dock on top, availableGeometry" << ret;
        break;
    case 1:   //右
    {
        int w = dockrect.left() - ret.left();
        if (w >= 0)
            ret.setWidth(static_cast<int>(w / ratio));   //原始大小计算的宽，需缩放处理
        else {
            fmCritical() << "dockrect.left() - ret.left() is invaild" << w;
        }
        fmDebug() << "dock on right,availableGeometry" << ret;
    } break;
    case 2:   //下
    {
        int h = dockrect.top() - ret.top();
        if (h >= 0)
            ret.setHeight(static_cast<int>(h / ratio));   //原始大小计算的高，需缩放处理
        else {
            fmCritical() << "dockrect.top() - ret.top() is invaild" << h;
        }
        fmDebug() << "dock on bottom,availableGeometry" << ret;
        break;
    }
    case 3:   //左
        ret.setX(dockrect.right());
        fmDebug() << "dock on left,availableGeometry" << ret;
        break;
    default:
        fmCritical() << "dock postion error!"
                     << "and  handleGeometry:" << hRect << "    dockrectI:" << dockrectI;
        break;
    }

    if (!checkAvailableGeometry(ret, geometry())) {
        fmCritical() << "available geometry may be wrong" << ret << "dock pos" << dockPos << dockrect
                     << "screen" << geometry() << hRect;
    }
    return ret;
}

QRect ScreenQt::handleGeometry() const
{
    return qscreen->handle()->geometry();
}

QScreen *ScreenQt::screen() const
{
    return qscreen;
}

bool ScreenQt::checkAvailableGeometry(const QRect &ava, const QRect &scr) const
{
    const qreal min = 0.8;
    if (((scr.height() * min) > ava.height())
        || ((scr.width() * min) > ava.width()))
        return false;
    return true;
}
