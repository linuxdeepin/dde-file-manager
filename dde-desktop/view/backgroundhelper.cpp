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

BackgroundHelper::BackgroundHelper(QWidget *parent)
    : QObject(parent)
    , windowManagerHelper(DWindowManagerHelper::instance())
{
    connect(windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
            this, &BackgroundHelper::onWMChanged);
    connect(windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
            this, &BackgroundHelper::onWMChanged);

    onWMChanged();
}

bool BackgroundHelper::isEnabled() const
{
    return windowManagerHelper->windowManagerName() != DWindowManagerHelper::OtherWM || !windowManagerHelper->hasComposite();
}

QLabel *BackgroundHelper::backgroundForScreen(QScreen *screen) const
{
    return backgroundMap.value(screen);
}

QList<QLabel *> BackgroundHelper::allBackgrounds() const
{
    return backgroundMap.values();
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
    if (isEnabled()) {
        if (wmInter) {
            return;
        }

        wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this);

        connect(wmInter, &WMInter::WorkspaceSwitched, this, [this] (int, int to) {
            currentWorkspaceIndex = to;
            updateBackground();
        });

        gsettings = new QGSettings("com.deepin.dde.appearance", "", this);

        connect(gsettings, &QGSettings::changed, this, [this] (const QString &key) {
            if (key == "backgroundUris") {
                updateBackground();
            }
        });

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
    const QSize trueSize = s->size() * s->devicePixelRatio();
    QPixmap pix = backgroundPixmap;

    pix = pix.scaled(trueSize,
                     Qt::KeepAspectRatioByExpanding,
                     Qt::SmoothTransformation);

    pix = pix.copy(QRect((pix.width() - trueSize.width()) / 2.0,
                         (pix.height() - trueSize.height()) / 2.0,
                         trueSize.width(),
                         trueSize.height()));

    pix.setDevicePixelRatio(l->devicePixelRatioF());
    l->setPixmap(pix);
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

    currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;
    backgroundPixmap = QPixmap(currentWallpaper);

    // 更新背景图
    for (QLabel *l : backgroundMap) {
        updateBackground(l);
    }
}

void BackgroundHelper::onScreenAdded(QScreen *screen)
{
    QLabel *l = new QLabel();

    backgroundMap[screen] = l;

    l->createWinId();
    l->windowHandle()->setScreen(screen);
    l->setGeometry(screen->geometry());

    Xcb::XcbMisc::instance().set_window_type(l->winId(), Xcb::XcbMisc::Desktop);

    l->show();

    connect(screen, &QScreen::geometryChanged, l, [l, this] (const QRect &geo) {
        l->setGeometry(geo);
        updateBackground(l);

        Q_EMIT backgroundGeometryChanged(l);
    });

    // 可能是由QGuiApplication引发的新屏幕添加，此处应该为新对象添加背景图
    updateBackground(l);

    Q_EMIT backgroundGeometryChanged(l);
}

void BackgroundHelper::onScreenRemoved(QScreen *screen)
{
    if (QLabel *l = backgroundMap.take(screen)) {
        Q_EMIT aboutDestoryBackground(l);

        l->deleteLater();
    }
}
