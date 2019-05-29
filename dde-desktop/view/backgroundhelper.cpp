/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "backgroundhelper.h"
#include "util/xcb/xcb.h"

#include <QScreen>
#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#define private public
#include <private/qhighdpiscaling_p.h>
#undef private

BackgroundHelper *BackgroundHelper::desktop_instance = nullptr;

BackgroundHelper::BackgroundHelper(bool preview, QObject *parent)
    : QObject(parent)
    , m_previuew(preview)
    , windowManagerHelper(DWindowManagerHelper::instance())
{
    if (!preview) {
        connect(windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
                this, &BackgroundHelper::onWMChanged);
        connect(windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
                this, &BackgroundHelper::onWMChanged);
        desktop_instance = this;
    }

    onWMChanged();
}

BackgroundHelper::~BackgroundHelper()
{
    for (QLabel *l : backgroundMap) {
        l->hide();
        l->deleteLater();
    }
}

BackgroundHelper* BackgroundHelper::getDesktopInstance()
{
    return desktop_instance;
}

bool BackgroundHelper::isEnabled() const
{
    // 只支持kwin，或未开启混成的桌面环境
    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
}

QLabel *BackgroundHelper::backgroundForScreen(QScreen *screen) const
{
    return backgroundMap.value(screen);
}

QList<QLabel *> BackgroundHelper::allBackgrounds() const
{
    return backgroundMap.values();
}

void BackgroundHelper::setBackground(const QString &path)
{
    qInfo() << "path:" << path;

    currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;
    backgroundPixmap = QPixmap(currentWallpaper);

    // 更新背景图
    for (QLabel *l : backgroundMap) {
        updateBackground(l);
    }
}

void BackgroundHelper::setVisible(bool visible)
{
    m_visible = visible;

    for (QLabel *l : backgroundMap) {
        l->setVisible(visible);
    }
}

bool BackgroundHelper::isKWin() const
{
    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM;
}

bool BackgroundHelper::isDeepinWM() const
{
    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::DeepinWM;
}

static bool wmDBusIsValid()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.wm");
}

void BackgroundHelper::onWMChanged()
{
    if (m_previuew || isEnabled()) {
        if (wmInter) {
            return;
        }

        wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this);
        gsettings = new QGSettings("com.deepin.dde.appearance", "", this);

        if (!m_previuew) {
            connect(wmInter, &WMInter::WorkspaceSwitched, this, [this] (int, int to) {
                currentWorkspaceIndex = to;
                updateBackground();
            });

            connect(gsettings, &QGSettings::changed, this, [this] (const QString &key) {
                if (key == "backgroundUris") {
                    updateBackground();
                }
            });
        }

        connect(qApp, &QGuiApplication::screenAdded, this, &BackgroundHelper::onScreenAdded);
        connect(qApp, &QGuiApplication::screenRemoved, this, &BackgroundHelper::onScreenRemoved);

        // 初始化窗口
        for (QScreen *s : qApp->screens()) {
            onScreenAdded(s);
        }

        // 初始化背景图
        updateBackground();
    } else {
        if (!wmInter) {
            return;
        }

        // 清理数据
        gsettings->deleteLater();
        gsettings = nullptr;

        wmInter->deleteLater();
        wmInter = nullptr;

        currentWallpaper.clear();
        currentWorkspaceIndex = 0;
        backgroundPixmap = QPixmap();

        disconnect(qApp, &QGuiApplication::screenAdded, this, &BackgroundHelper::onScreenAdded);
        disconnect(qApp, &QGuiApplication::screenRemoved, this, &BackgroundHelper::onScreenRemoved);

        // 销毁窗口
        for (QScreen *s : backgroundMap.keys()) {
            onScreenRemoved(s);
        }
    }

    Q_EMIT enableChanged();
}

void BackgroundHelper::updateBackground(QLabel *l)
{
    if (backgroundPixmap.isNull())
        return;

    QScreen *s = l->windowHandle()->screen();
    l->windowHandle()->handle()->setGeometry(s->handle()->geometry());

    const QSize trueSize = s->handle()->geometry().size();
    QPixmap pix = backgroundPixmap;

    pix = pix.scaled(trueSize,
                     Qt::KeepAspectRatioByExpanding,
                     Qt::SmoothTransformation);

    if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
        pix = pix.copy(QRect((pix.width() - trueSize.width()) / 2.0,
                             (pix.height() - trueSize.height()) / 2.0,
                             trueSize.width(),
                             trueSize.height()));
    }

    pix.setDevicePixelRatio(l->devicePixelRatioF());
    l->setPixmap(pix);

    qInfo() << l->windowHandle()->screen() << currentWallpaper << pix;
}

void BackgroundHelper::updateBackground()
{
    QString path = wmDBusIsValid() ? wmInter->GetCurrentWorkspaceBackground() : QString();

    if (path.isEmpty()
            // 调用失败时会返回 "The name com.deepin.wm was not provided by any .service files"
            // 此时 wmInter->isValid() = true, 且 dubs last error type 为 NoError
            || (!path.startsWith("/") && !path.startsWith("file:"))) {
        path = gsettings->get("background-uris").toStringList().value(currentWorkspaceIndex);

        if (path.isEmpty())
            return;
    }

    setBackground(path);
}

void BackgroundHelper::onScreenAdded(QScreen *screen)
{
    QLabel *l = new QLabel();

    backgroundMap[screen] = l;

    l->createWinId();
    l->windowHandle()->setScreen(screen);
    l->setGeometry(screen->geometry());

    QTimer::singleShot(0, this, [l, screen] {
        // 禁用高分屏缩放，防止窗口的sizeIncrement默认设置大于1
        bool hi_active = QHighDpiScaling::m_active;
        QHighDpiScaling::m_active = false;
        l->windowHandle()->handle()->setGeometry(screen->handle()->geometry());
        QHighDpiScaling::m_active = hi_active;
    });

    if (m_previuew) {
        l->setWindowFlags(l->windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    } else {
        Xcb::XcbMisc::instance().set_window_type(l->winId(), Xcb::XcbMisc::Desktop);
    }

    if (m_visible)
        l->show();
    else
        qDebug() << "Disable show the background widget, of screen:" << screen << screen->geometry();

    connect(screen, &QScreen::geometryChanged, l, [l, this, screen] () {
        qDebug() << "screen geometry changed:" << screen << screen->geometry();

        // 因为接下来会发出backgroundGeometryChanged信号，
        // 所以此处必须保证QWidget::geometry的值和接下来对其windowHandle()对象设置的geometry一致
        l->setGeometry(screen->geometry());

        // 忽略屏幕缩放，设置窗口的原始大小
        // 调用此函数后不会立即更新QWidget::geometry，而是在收到窗口resize事件后更新
        bool hi_active = QHighDpiScaling::m_active;
        QHighDpiScaling::m_active = false;
        l->windowHandle()->handle()->setGeometry(screen->handle()->geometry());
        QHighDpiScaling::m_active = hi_active;
        updateBackground(l);

        Q_EMIT backgroundGeometryChanged(l);
    });

    // 可能是由QGuiApplication引发的新屏幕添加，此处应该为新对象添加背景图
    updateBackground(l);

    Q_EMIT backgroundGeometryChanged(l);
    Q_EMIT backgroundAdded(l);

    qInfo() << screen << screen->geometry();
}

void BackgroundHelper::onScreenRemoved(QScreen *screen)
{
    if (QLabel *l = backgroundMap.take(screen)) {
        Q_EMIT aboutDestoryBackground(l);

        l->deleteLater();
    }

    qInfo() << screen;
}
