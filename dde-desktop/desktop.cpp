/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "desktop.h"

#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDBusConnection>
#include <QScreen>

#include <durl.h>

#include "view/canvasgridview.h"
#include "view/backgroundhelper.h"
#include "presenter/apppresenter.h"
#include "presenter/display.h"

#include "../dde-wallpaper-chooser/frame.h"
#include "../util/dde/desktopinfo.h"

#ifndef DISABLE_ZONE
#include "../dde-zone/mainwindow.h"
#endif

#include "util/xcb/xcb.h"
#include "backgroundmanager.h"
#include "canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "presenter/gridmanager.h"

using WallpaperSettings = Frame;

extern QScreen *GetPrimaryScreen();

#ifndef DISABLE_ZONE
using ZoneSettings = ZoneMainWindow;
#endif

class DesktopPrivate
{
public:
#if USINGOLD
    CanvasGridView      screenFrame;
    BackgroundHelper *background = nullptr;
#else
    BackgroundManager *m_background = nullptr;
    CanvasViewManager *m_canvas = nullptr;
#endif
    WallpaperSettings *wallpaperSettings{ nullptr };

#ifndef DISABLE_ZONE
    ZoneSettings *zoneSettings { nullptr };
#endif
};

Desktop::Desktop()
    : d(new DesktopPrivate)
{
#if USINGOLD
    d->background = new BackgroundHelper();
    DesktopInfo desktoInfo;
    connect(d->background, &BackgroundHelper::enableChanged, this, &Desktop::onBackgroundEnableChanged);
    if (desktoInfo.waylandDectected()) {
        connect(Display::instance(), &Display::primaryScreenChanged, this, &Desktop::onBackgroundEnableChanged);
    } else {
        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &Desktop::onBackgroundEnableChanged);
    }


    connect(d->background, &BackgroundHelper::aboutDestoryBackground, this, [this] (QWidget * l) {
        if (l == d->screenFrame.parent()) {
            d->screenFrame.setParent(nullptr);
        }
    }, Qt::DirectConnection);
    // 任意控件改变位置都可能会引起主窗口被其它屏幕上的窗口所遮挡
    connect(d->background, &BackgroundHelper::backgroundGeometryChanged, this, &Desktop::onBackgroundGeometryChanged);
    onBackgroundEnableChanged();
#else
    d->m_background = new BackgroundManager;
#endif
}

Desktop::~Desktop()
{

}

static void setWindowFlag(QWidget *w, Qt::WindowType flag, bool on)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    if (on) {
        w->setWindowFlags(w->windowFlags() | flag);
    } else {
        w->setWindowFlags(w->windowFlags() & ~flag);
    }
#else
    w->setWindowFlag(flag, on);
#endif
}

#if USINGOLD
void Desktop::onBackgroundEnableChanged()
{
    qInfo() << "Primary Screen:" << qApp->primaryScreen();
    qInfo() << "Background enabled:" << d->background->isEnabled();

    if (d->background->isEnabled()) {

        //if X11
        //QWidget *background = d->background->backgroundForScreen(qApp->primaryScreen());
        //else
        QWidget *background;

        auto e = QProcessEnvironment::systemEnvironment();
        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

        if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
                WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {

            background = d->background->backgroundForScreen(GetPrimaryScreen());
        } else {
            background = d->background->backgroundForScreen(qApp->primaryScreen());
        }

        d->screenFrame.setAttribute(Qt::WA_NativeWindow, false);
        d->screenFrame.setParent(background);
        d->screenFrame.move(0, 0);
        d->screenFrame.show();

        // 防止复制模式下主屏窗口被遮挡
        background->activateWindow();
        QMetaObject::invokeMethod(background, "raise", Qt::QueuedConnection);

        // 隐藏完全重叠的窗口
        for (QWidget *l : d->background->allBackgrounds()) {
            if (l != background) {
                Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), true);
                l->setVisible(!background->geometry().contains(l->geometry()));
            } else {
                Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), false);
                l->show();
            }
        }

        //if X11
        //nothing
        //else

    } else {
        d->screenFrame.setParent(nullptr);
        setWindowFlag(&d->screenFrame, Qt::FramelessWindowHint, true);

        //if X11
        //d->screenFrame.QWidget::setGeometry(qApp->primaryScreen()->geometry());
        //else


        auto e = QProcessEnvironment::systemEnvironment();
        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

        if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
                WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            d->screenFrame.QWidget::setGeometry(Display::instance()->primaryRect());
        }

        else {
            d->screenFrame.QWidget::setGeometry(qApp->primaryScreen()->geometry());
        }


        Xcb::XcbMisc::instance().set_window_type(d->screenFrame.winId(), Xcb::XcbMisc::Desktop);

        d->screenFrame.show();
    }
}

void Desktop::onBackgroundGeometryChanged(QWidget *l)
{
    d->background->resetBackgroundVisibleState();

    QWidget *primaryBackground = d->screenFrame.parentWidget();

    if (!primaryBackground) {
        return;
    }

    qInfo() << "primaryBackground widget geometry: " << primaryBackground->geometry()
            << "changedBackground widget geometry:" << l->geometry();

    primaryBackground->activateWindow();
    QMetaObject::invokeMethod(primaryBackground, "raise", Qt::QueuedConnection);

    if (l != primaryBackground && primaryBackground->geometry().contains(l->geometry())) {
        l->hide();
    } else {
        l->show();
    }
}
#endif
void Desktop::loadData()
{
    Presenter::instance()->init();
}

void Desktop::loadView()
{
#if USINGOLD //old;
    d->screenFrame.initRootUrl();
#else
    d->m_canvas = new CanvasViewManager(d->m_background);
#endif
}

void Desktop::showWallpaperSettings(QString name, int mode)
{
    if(name.isNull() || name.isEmpty()){
        if (ScreenHelper::screenManager()->primaryScreen() == nullptr){
            qCritical() << "get primary screen failed! stop show wallpaper";
            return;
        }

        name = ScreenHelper::screenManager()->primaryScreen()->name();
    }

    if (d->wallpaperSettings) {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    }

    d->wallpaperSettings = new WallpaperSettings(name, Frame::Mode(mode));
    connect(d->wallpaperSettings, &Frame::done, this, [ = ] {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    });
    connect(d->wallpaperSettings, &Frame::aboutHide, this, [this] {
        //const QString &desktopImage = d->wallpaperSettings->desktopBackground(); /*** old code ***/
        QPair<QString, QString> screenImage = d->wallpaperSettings->desktopBackground();

#if 0
        if (!desktopImage.isEmpty())
            d->background->setBackground(desktopImage);
#else
            d->m_background->setBackgroundImage(screenImage.first, screenImage.second);
#endif
    }, Qt::DirectConnection);
#if 0
    //屏幕有改变直接退出壁纸设置
    auto close = [this](){
        qDebug() << "screen changed , wallpaperSettings is exiting";
        if (d->wallpaperSettings) {
            d->wallpaperSettings->close();
            d->wallpaperSettings->deleteLater();
            d->wallpaperSettings = nullptr;
        }
    };
    connect(ScreenMrg, &AbstractScreenManager::sigScreenChanged,[close](){
            close();
    });
    connect(ScreenMrg, &AbstractScreenManager::sigScreenGeometryChanged,[close](){
            close();
    });
    //end
#endif
    d->wallpaperSettings->show();
    d->wallpaperSettings->grabKeyboard();
}

#ifndef DISABLE_ZONE
void Desktop::showZoneSettings()
{
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
}

#endif

void Desktop::initDebugDBus(QDBusConnection &conn)
{
#if USINGOLD
    if (!conn.registerObject(DesktopCanvasPath, &d->screenFrame,
                             QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "registerObject Failed" << conn.lastError();
        exit(0x0004);
    }
#endif
}

void Desktop::EnableUIDebug(bool enable)
{
    for (CanvasViewPointer view: d->m_canvas->canvas().values()){
        view->EnableUIDebug(enable);
        view->update();
    }
}

void Desktop::Reset()
{
    ScreenMrg->reset();
    if (d->m_background->isEnabled()){
        d->m_background->onBackgroundBuild();
    }
    else {
        d->m_background->onSkipBackgroundBuild();
    }
}

void Desktop::ShowInfo()
{
    ScreenPointer primary = ScreenMrg->primaryScreen();
    qInfo() << "**************Desktop Info" << qApp->applicationVersion()
            << "*****************";
    if (primary)
        qInfo() << "primary screen :" << primary->name()
                << "available geometry" << primary->availableGeometry()
                << "screen count" << ScreenMrg->screens().count();
    else
        qCritical() << "primary screen error! not found";

    qInfo() << "*****************Screens  Mode " << ScreenMrg->displayMode()
            << "********************";
    int num = 1;
    for (ScreenPointer screen : ScreenMrg->logicScreens()){
        if (screen){
            qInfo() << screen.get() << "screen name " << screen->name()
                    << "num" << num << "geometry" << screen->geometry();
            ++num;
        }else {
            qCritical() << "error! empty screen pointer!";
        }
    }

    qInfo() << "*****************Background Eable" << d->m_background->isEnabled()
            << "**********************";
    auto backgronds = d->m_background->allbackgroundWidgets();
    for (auto iter = backgronds.begin(); iter != backgronds.end(); ++iter) {
        qInfo() << "Background" << iter.value().get() << "on screen" << iter.key()->name() << iter.key().get()
                << "geometry" << iter.value()->geometry() << "visable" << iter.value()->isVisible();
    }

    qInfo() << "*****************Canvas Grid" << "**********************";
    if (d->m_canvas){
        auto canvas = d->m_canvas->canvas();
        GridCore *core = GridManager::instance()->core();
        for (auto iter = canvas.begin(); iter != canvas.end(); ++iter){
            int num = iter.value()->screenNum();
            qInfo() << "canvas" << iter.value().get() << "on screen" << iter.value()->canvansScreenName()
                    << "num" << num << "geometry" << iter.value()->geometry()
                    << "background" << iter.value()->parentWidget() << "screen" << iter.key().get();
            if (core->screensCoordInfo.contains(num)){
                auto coord = core->screensCoordInfo.value(num);
                qInfo() << "coord " << coord.first << "*" << coord.second
                        << "display items count" << core->itemGrids.value(num).size();
            }
            else {
                qCritical() << "Grid" << iter.value()->screenNum() << "not find coordinfo";
            }

        }

        qInfo() << "overlap items count" << core->overlapItems.size();
        delete core;
    }
    else {
        qWarning() << "not load canvasgridview";
    }
    qInfo() << "************Desktop Infomation End **************";
}

void Desktop::ShowWallpaperChooser(const QString &screen)
{
    showWallpaperSettings(screen,Frame::WallpaperMode);
}

void Desktop::ShowScreensaverChooser(const QString &screen)
{
    showWallpaperSettings(screen,Frame::ScreenSaverMode);
}

#if USINGOLD
#ifdef QT_DEBUG
void Desktop::logAllScreenLabel()
{
    if (d->background)
        d->background->printLog();
}

void Desktop::logScreenLabel(int index)
{
    if (d->background)
        d->background->printLog(index);
}

void Desktop::mapLabelScreen(int labelIndex, int screenIndex)
{
    if (d->background)
        d->background->mapLabelScreen(labelIndex, screenIndex);
}
#endif
#endif // QT_DEBUG
