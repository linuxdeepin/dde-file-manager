// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    // 处理缩放，先不考虑高分屏的特殊处理
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    if (ratio != 1.0)
        orgRect = QRect(orgRect.x() / ratio, orgRect.y() / ratio, static_cast<int>(orgRect.width() / ratio), static_cast<int>(orgRect.height() / ratio));
    return orgRect;
}
}   // namespace GlobalPrivate

DDPCORE_USE_NAMESPACE

ScreenQt::ScreenQt(QScreen *screen, QObject *parent)
    : DFMBASE_NAMESPACE::AbstractScreen(parent), qscreen(screen)
{
    Q_ASSERT(qscreen);

    fmDebug() << "ScreenQt created for screen:" << screen->name() << "geometry:" << screen->geometry();

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
    // return m_screen->availableGeometry();
    // end

    QRect ret = geometry();   // 已经缩放过

    if (!DBusHelper::isDockEnable()) {
        fmWarning() << "DDE dock is not registered, using full screen geometry";
        return ret;
    }

    int dockHideMode = DockInfoIns->hideMode();
    if (1 == dockHideMode) {   // 隐藏
        fmDebug() << "Dock is hidden, using full screen geometry";
        return ret;
    }

    DockRect dockrectI = DockInfoIns->frontendWindowRect();   // 原始dock大小
    const QRect dockrect = GlobalPrivate::dealRectRatio(dockrectI.operator QRect());   // 缩放处理

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    const QRect hRect = handleGeometry();
    const QRect hRectScaled = GlobalPrivate::dealRectRatio(hRect);   // 将物理坐标转换为逻辑坐标

    if (!hRect.contains(dockrectI) && !ret.contains(dockrect)) {
        QRect orgDockRect(dockrectI);
        fmDebug() << "Dock not contained in screen geometry, using full screen - screen:" << name()
                  << "handleGeometry:" << hRect << "originalDockRect:" << orgDockRect
                  << "scaledDockRect:" << dockrect << "ratio:" << ratio;
        return ret;
    }

    const int dockPos = DockInfoIns->position();
    fmDebug() << "Adjusting available geometry for dock position:" << dockPos << "on screen:" << name();

    switch (dockPos) {
    case 0:   // 上
    {
        // 计算dock底部相对于屏幕顶部的偏移（逻辑坐标）
        int yOffset = dockrect.bottom() - hRectScaled.top();
        ret.setY(ret.y() + yOffset);
        fmDebug() << "Dock on top, adjusted available geometry:" << ret;
        break;
    }
    case 1:   // 右
    {
        // 计算dock左侧相对于屏幕左侧的距离（逻辑坐标）
        int w = dockrect.left() - hRectScaled.left();
        if (w >= 0) {
            ret.setWidth(w);
        } else {
            fmCritical() << "Invalid width calculation for right dock:" << w
                         << "dockLeft:" << dockrect.left() << "screenLeft:" << hRectScaled.left();
        }
        fmDebug() << "Dock on right, adjusted available geometry:" << ret;
    } break;
    case 2:   // 下
    {
        // 计算dock顶部相对于屏幕顶部的距离（逻辑坐标）
        int h = dockrect.top() - hRectScaled.top();
        if (h >= 0) {
            ret.setHeight(h);
        } else {
            fmCritical() << "Invalid height calculation for bottom dock:" << h
                         << "dockTop:" << dockrect.top() << "screenTop:" << hRectScaled.top();
        }
        fmDebug() << "Dock on bottom, adjusted available geometry:" << ret;
        break;
    }
    case 3:   // 左
    {
        // 计算dock右侧相对于屏幕左侧的偏移（逻辑坐标）
        int xOffset = dockrect.right() - hRectScaled.left();
        ret.setX(ret.x() + xOffset);
        fmDebug() << "Dock on left, adjusted available geometry:" << ret;
        break;
    }
    default:
        fmCritical() << "Invalid dock position:" << dockPos
                     << "handleGeometry:" << hRect << "dockRect:" << dockrectI;
        break;
    }

    if (!checkAvailableGeometry(ret, geometry())) {
        fmCritical() << "Available geometry validation failed - calculated:" << ret
                     << "dockPosition:" << dockPos << "dockRect:" << dockrect
                     << "screenGeometry:" << geometry() << "handleGeometry:" << hRect;
    } else {
        fmDebug() << "Available geometry calculated successfully:" << ret << "for screen:" << name();
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
    bool valid = true;
    if ((scr.height() * min) > ava.height()) {
        fmWarning() << "Available height too small - screen height:" << scr.height()
                    << "available height:" << ava.height() << "minimum ratio:" << min;
        valid = false;
    }

    if ((scr.width() * min) > ava.width()) {
        fmWarning() << "Available width too small - screen width:" << scr.width()
                    << "available width:" << ava.width() << "minimum ratio:" << min;
        valid = false;
    }

    if (valid) {
        fmDebug() << "Available geometry validation passed for screen:" << qscreen->name();
    }

    return valid;
}
