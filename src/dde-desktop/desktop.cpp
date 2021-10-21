/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "desktop.h"
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDBusConnection>
#include <QScreen>
#include "desktopprivate.h"
#include "screen/screenhelper.h"
#include "presenter/gridmanager.h"
#include "../util/dde/desktopinfo.h"
#include "presenter/apppresenter.h"
#include "canvasviewmanager.h"
#include "backgroundmanager.h"
#include "desktopprivate.h"

Desktop::Desktop()
    : d(new DesktopPrivate)
{
}

Desktop::~Desktop()
{

}

void Desktop::preInit()
{
    d->m_background = new BackgroundManager;
    //    //5s归还一次内存
    //    QTimer *releaseMem = new QTimer;
    //    connect(releaseMem,&QTimer::timeout,this,[](){malloc_trim(0);});
    //    releaseMem->start(5000);
}

void Desktop::loadData()
{
    Presenter::instance()->init();
}

void Desktop::loadView()
{
    d->m_canvas = new CanvasViewManager(d->m_background);
}

void Desktop::showWallpaperSettings(QString name, int mode)
{
    if (name.isNull() || name.isEmpty()) {
        if (ScreenHelper::screenManager()->primaryScreen() == nullptr) {
            qCritical() << "get primary screen failed! stop show wallpaper";
            return;
        }

        name = ScreenHelper::screenManager()->primaryScreen()->name();
    }

    if (d->wallpaperSettings) {
        //防止暴力操作，高频调用接口
        if (d->wallpaperSettings->isVisible())
            return;
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    }

    d->wallpaperSettings = new WallpaperSettings(name, Frame::Mode(mode));
    connect(d->wallpaperSettings, &Frame::done, this, [ = ] {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    });


    connect(d->wallpaperSettings, &Frame::backgroundChanged, this, [this] {
        WallpaperSettings *setting = dynamic_cast<WallpaperSettings *>(sender());
        if (setting && d->m_background) {
            QPair<QString, QString> screenImage = setting->desktopBackground();
            d->m_background->setBackgroundImage(screenImage.first, screenImage.second);
        }
    }, Qt::DirectConnection);

    d->wallpaperSettings->show();

    //使壁纸设置窗口保持焦点
    auto autoAct = new AutoActivateWindow(d->wallpaperSettings);
    autoAct->setWatched(d->wallpaperSettings);
    autoAct->start();
}


void Desktop::showZoneSettings()
{
#ifndef DISABLE_ZONE
    if (d->zoneSettings) {
        d->zoneSettings->deleteLater();
        d->zoneSettings = nullptr;
    }

    d->zoneSettings = new ZoneSettings;
    connect(d->zoneSettings, &ZoneMainWindow::finished, this, [ = ] {
        d->zoneSettings->deleteLater();
        d->zoneSettings = nullptr;
    });

    d->zoneSettings->show();
    d->zoneSettings->grabKeyboard();
#else
    qWarning() << "Zone is disabled";
#endif
}


void Desktop::EnableUIDebug(bool enable)
{
    for (CanvasViewPointer view : d->m_canvas->canvas().values()) {
        view->EnableUIDebug(enable);
        view->update();
    }
}

void Desktop::SetVisible(int screenNum, bool v)
{
    --screenNum;
    QVector<ScreenPointer> screens = ScreenMrg->logicScreens();
    if (screens.size() > screenNum && screenNum >= 0) {
        ScreenPointer sp = screens[screenNum];
        BackgroundWidgetPointer bw = d->m_background->allbackgroundWidgets().value(sp);
        if (bw)
            bw->setVisible(v);

        CanvasViewPointer view = d->m_canvas->canvas().value(sp);
        if (view)
            view->setVisible(v);
    }
}

void Desktop::FixGeometry(int screenNum)
{
    --screenNum;
    QVector<ScreenPointer> screens = ScreenMrg->logicScreens();
    if (screens.size() > screenNum && screenNum >= 0) {
        ScreenPointer sp = screens[screenNum];
        emit ScreenMrg->sigScreenGeometryChanged();
    }
}

void Desktop::Reset()
{
    ScreenMrg->reset();
    if (d->m_background->isEnabled()) {
        d->m_background->onBackgroundBuild();
    } else {
        d->m_background->onSkipBackgroundBuild();
    }
}

void Desktop::PrintInfo()
{
    ScreenPointer primary = ScreenMrg->primaryScreen();
    qInfo() << "**************Desktop Info" << qApp->applicationVersion()
            << "*****************";
    if (primary)
        qInfo() << "primary screen :" << primary->name()
                << "available geometry" << primary->availableGeometry()
                << "handle geometry"   << primary->handleGeometry()
                << "devicePixelRatio" << ScreenMrg->devicePixelRatio()
                << "screen count" << ScreenMrg->screens().count();
    else
        qCritical() << "primary screen error! not found";

    qInfo() << "*****************Screens  Mode " << ScreenMrg->displayMode()
            << "********************";
    qInfo() << "*****************last changed  Mode " << ScreenMrg->lastChangedMode()
            << "********************";
    int num = 1;
    for (ScreenPointer screen : ScreenMrg->logicScreens()) {
        if (screen) {
            qInfo() << screen.get() << "screen name " << screen->name()
                    << "num" << num << "geometry" << screen->geometry()
                    << "handle geometry"   << screen->handleGeometry();
            ++num;
        } else {
            qCritical() << "error! empty screen pointer!";
        }
    }

    qInfo() << "*****************Background Eable" << d->m_background->isEnabled()
            << "**********************";
    auto backgronds = d->m_background->allbackgroundWidgets();
    for (auto iter = backgronds.begin(); iter != backgronds.end(); ++iter) {
        qInfo() << "Background" << iter.value().get() << "on screen" << iter.key()->name() << iter.key().get()
                << "geometry" << iter.value()->geometry() << "visable" << iter.value()->isVisible()
                << "rect" << iter.value()->rect() << "background image"
                << d->m_background->backgroundImages().value(iter.key()->name())
                << "pixmap" << iter.value()->pixmap();

        if (iter.value()->windowHandle()) {
            qInfo() << "window geometry" << iter.value()->windowHandle()->geometry()
                    << iter.value()->windowHandle()->screen()->geometry();
        }
    }

    qInfo() << "*****************Canvas Grid" << "**********************";
    if (d->m_canvas) {
        auto canvas = d->m_canvas->canvas();
        GridCore *core = GridManager::instance()->core();
        for (auto iter = canvas.begin(); iter != canvas.end(); ++iter) {
            int screenNum = iter.value()->screenNum();
            qInfo() << "canvas" << iter.value().get() << "on screen" << iter.value()->canvansScreenName()
                    << "num" << screenNum << "geometry" << iter.value()->geometry()
                    << "background" << iter.value()->parentWidget() << "screen" << iter.key().get();
            if (core->screensCoordInfo.contains(screenNum)) {
                auto coord = core->screensCoordInfo.value(screenNum);
                qInfo() << "coord " << coord.first << "*" << coord.second
                        << "display items count" << core->itemGrids.value(screenNum).size();
            } else {
                qCritical() << "Grid" << iter.value()->screenNum() << "not find coordinfo";
            }

        }

        qInfo() << "overlap items count" << core->overlapItems.size();
        delete core;
    } else {
        qWarning() << "not load canvasgridview";
    }
    qInfo() << "************Desktop Infomation End **************";
}

void Desktop::Refresh()
{
    if (d->m_canvas)
        for (CanvasViewPointer view : d->m_canvas->canvas().values()) {
            view->Refresh();
        }
}

void Desktop::ShowWallpaperChooser(const QString &screen)
{
    showWallpaperSettings(screen, Frame::WallpaperMode);
}

void Desktop::ShowScreensaverChooser(const QString &screen)
{
#ifndef DISABLE_SCREENSAVER
    showWallpaperSettings(screen, Frame::ScreenSaverMode);
#endif
}

QList<int> Desktop::GetIconSize()
{
    QSize iconSize{0, 0};
    if (d->m_canvas && !d->m_canvas->canvas().isEmpty())
        iconSize = d->m_canvas->canvas().first()->iconSize();
    QList<int> size{iconSize.width(), iconSize.height()};
    return  size;
}
