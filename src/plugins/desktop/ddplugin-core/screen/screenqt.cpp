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

    DockRect dockrectI = DockInfoIns->frontendWindowRect();

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    const QRect hRect = handleGeometry();

    if (!hRect.contains(dockrectI)) {
        fmDebug() << "Dock not contained in screen geometry, using full screen - screen:" << name()
                  << "handleGeometry:" << hRect << "dockRect:" << QRect(dockrectI)
                  << "ratio:" << ratio;
        return ret;
    }

    const int dockPos = DockInfoIns->position();
    fmDebug() << "Adjusting available geometry for dock position:" << dockPos << "on screen:" << name();

    // 使用dock物理尺寸直接计算，避免dock坐标(屏幕本地坐标系)与屏幕geometry(虚拟桌面坐标系)
    // 在不同坐标系下的位置错位问题。ret=geometry()已经是正确的屏幕区域，只需减去dock尺寸。
    switch (dockPos) {
    case 0: {   // 上
        int dockH = static_cast<int>(dockrectI.height / ratio);
        ret.adjust(0, dockH, 0, 0);
        fmDebug() << "Dock on top, adjusted available geometry:" << ret;
        break;
    }
    case 1: {   // 右
        int dockW = static_cast<int>(dockrectI.width / ratio);
        ret.adjust(0, 0, -dockW, 0);
        fmDebug() << "Dock on right, adjusted available geometry:" << ret;
        break;
    }
    case 2: {   // 下
        int dockH = static_cast<int>(dockrectI.height / ratio);
        ret.adjust(0, 0, 0, -dockH);
        fmDebug() << "Dock on bottom, adjusted available geometry:" << ret;
        break;
    }
    case 3: {   // 左
        int dockW = static_cast<int>(dockrectI.width / ratio);
        ret.adjust(dockW, 0, 0, 0);
        fmDebug() << "Dock on left, adjusted available geometry:" << ret;
        break;
    }
    default:
        fmCritical() << "Invalid dock position:" << dockPos
                     << "handleGeometry:" << hRect << "dockRect:" << QRect(dockrectI);
        break;
    }

    if (!checkAvailableGeometry(ret, geometry())) {
        fmCritical() << "Available geometry validation failed - calculated:" << ret
                     << "dockPosition:" << dockPos << "dockRect:" << QRect(dockrectI)
                     << "screenGeometry:" << geometry() << "handleGeometry:" << hRect;
        ret = geometry();   // 校验失败时回退到全屏几何，不减dock
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
