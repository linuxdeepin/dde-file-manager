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

//#include "util/xcb/xcb.h"
#include "util/util.h"

using WallpaperSettings = Frame;

extern QScreen *GetPrimaryScreen();

#ifndef DISABLE_ZONE
using ZoneSettings = ZoneMainWindow;
#endif

class DesktopPrivate
{
public:
    CanvasGridView      screenFrame;
    BackgroundHelper *background = nullptr;
    WallpaperSettings *wallpaperSettings{ nullptr };

#ifndef DISABLE_ZONE
    ZoneSettings *zoneSettings { nullptr };
#endif
};

Desktop::Desktop()
    : d(new DesktopPrivate)
{
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

void Desktop::onBackgroundEnableChanged()
{
    if (DesktopInfo().waylandDectected()) {
        qInfo() << "Primary Screen:" << Display::instance()->primaryName();
        if (d->background->isEnabled()) {

            //更新显示区域
            d->background->monitorRectChanged();

            QWidget *background = d->background->waylandBackground(Display::instance()->primaryName());
            if(!background)
            {
                qWarning()<<"Warning:cannot find paimary widget and screen name:"<<Display::instance()->primaryName();
                return;
            }

            d->screenFrame.setAttribute(Qt::WA_NativeWindow, false);
            d->screenFrame.setParent(background);
            d->screenFrame.move(0, 0);
            d->screenFrame.show();

            // 防止复制模式下主屏窗口被遮挡
            background->activateWindow();
            QMetaObject::invokeMethod(background, "raise", Qt::QueuedConnection);

            // 隐藏完全重叠的窗口
            for (QWidget *l : d->background->waylandAllBackgrounds()) {
                if (l != background) {
                    //Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), true);
                    l->setWindowFlag(Qt::WindowTransparentForInput);
                    l->setVisible(!background->geometry().contains(l->geometry()));
                } else {
                   //Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), false);
                   l->setWindowFlag(Qt::WindowTransparentForInput,false);
                   l->show();
                }

                qDebug() << "hide overlap widget" << l << l->isVisible() << l->geometry();
            }
        } else {
            d->screenFrame.setParent(nullptr);
            setWindowFlag(&d->screenFrame, Qt::FramelessWindowHint, true);
            d->screenFrame.QWidget::setGeometry(Display::instance()->primaryRect());
            DesktopUtil::set_desktop_window(&d->screenFrame);
            d->screenFrame.show();
        }

        return;
    }
    qInfo() << "Primary Screen:" << qApp->primaryScreen();
    qInfo() << "Background enabled:" << d->background->isEnabled();

    if (d->background->isEnabled()) {

        //if X11
        //QWidget *background = d->background->backgroundForScreen(qApp->primaryScreen());
        //else
        QWidget *background;
        if (DesktopInfo().waylandDectected()) {

            background = d->background->backgroundForScreen(GetPrimaryScreen());

        } else {
            background = d->background->backgroundForScreen(qApp->primaryScreen());
        }

        if(!background)
        {
            qWarning()<<"Warning:cannot find paimary widget and screen name:"<<Display::instance()->primaryName();
            return;
        }

        d->screenFrame.setAttribute(Qt::WA_NativeWindow, false);
        d->screenFrame.setParent(background);
        d->screenFrame.move(0, 0);
        d->screenFrame.show();

        // 防止复制模式下主屏窗口被遮挡
        background->activateWindow();
        QMetaObject::invokeMethod(background, "raise", Qt::QueuedConnection);

        // 隐藏完全重叠的窗口
        qDebug() << "primary background" << background << background->isVisible() << background->geometry();
        for (QWidget *l : d->background->allBackgrounds()) {
            if (l != background) {
                //Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), true);
                l->setWindowFlag(Qt::WindowTransparentForInput);
                //由于之前的BackgroundLabel::setVisible中有做特殊情况下强制显示的操作，所以这里暂时这样处理
                //不是是否会会有i其他影响
                l->setVisible(!background->geometry().contains(l->geometry()));
            } else {
                //Xcb::XcbMisc::instance().set_window_transparent_input(l->winId(), false);
                l->setWindowFlag(Qt::WindowTransparentForInput, false);
                l->show();
            }
            qDebug() << "hide overlap widget" << l << l->isVisible() << l->geometry();
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

        if (DesktopInfo().waylandDectected()) {
            d->screenFrame.QWidget::setGeometry(Display::instance()->primaryRect());
        }
        else {
            d->screenFrame.QWidget::setGeometry(qApp->primaryScreen()->geometry());
        }

        DesktopUtil::set_desktop_window(&d->screenFrame);
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

void Desktop::loadData()
{
    Presenter::instance()->init();
}

void Desktop::loadView()
{
    d->screenFrame.initRootUrl();
}

void Desktop::showWallpaperSettings(int mode)
{
    if (d->wallpaperSettings) {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    }

    d->wallpaperSettings = new WallpaperSettings(Frame::Mode(mode));
    connect(d->wallpaperSettings, &Frame::done, this, [ = ] {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    });
    connect(d->wallpaperSettings, &Frame::aboutHide, this, [this] {
        WallpaperSettings *set = dynamic_cast<WallpaperSettings *>(sender());
        if (set){
            QString desktopImage = set->desktopBackground();
            if (!desktopImage.isEmpty())
                d->background->setBackground(desktopImage);
        }

    }, Qt::DirectConnection);

    d->wallpaperSettings->show();
    //d->wallpaperSettings->grabKeyboard(); //设计按键交互功能QWindow *window = d->wallpaperSettings->windowHandle();
    //监控窗口状态
    QWindow *window = d->wallpaperSettings->windowHandle();
    connect(window, &QWindow::activeChanged, d->wallpaperSettings, [=]() {
        if(d->wallpaperSettings == nullptr || d->wallpaperSettings->isActiveWindow())
            return;
        //激活窗口
        d->wallpaperSettings->activateWindow();
        //10毫秒后再次检测
        QTimer::singleShot(10,d->wallpaperSettings,[=]()
        {
            if (d->wallpaperSettings && !d->wallpaperSettings->isActiveWindow())
                d->wallpaperSettings->activateWindow();
        });
    });
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
    if (!conn.registerObject(DesktopCanvasPath, &d->screenFrame,
                             QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "registerObject Failed" << conn.lastError();
        exit(0x0004);
    }
}

CanvasGridView *Desktop::getView()
{
    return (&(d->screenFrame));
}

void Desktop::Show()
{
    d->screenFrame.show();
}

void Desktop::ShowWallpaperChooser()
{
    showWallpaperSettings(Frame::WallpaperMode);
}

void Desktop::ShowScreensaverChooser()
{
    showWallpaperSettings(Frame::ScreenSaverMode);
}

void Desktop::Refresh()
{
    d->screenFrame.Refresh();
}

#ifdef QT_DEBUG
void Desktop::logAllScreenLabel()
{
    if (d->background)
        d->background->printLog();

    qDebug() << "canvas geometry" << d->screenFrame.geometry();
    if (d->screenFrame.windowHandle())
    qDebug()<< d->screenFrame.windowHandle()->geometry() << d->screenFrame.windowHandle()->screen()->name()
    << d->screenFrame.windowHandle()->screen()->geometry();


    if (d->screenFrame.parentWidget()){
        qDebug() <<d->screenFrame.parentWidget() << d->screenFrame.parentWidget()->geometry()
                << d->screenFrame.parentWidget()->windowHandle()->screen()->name();
    }else {
        qDebug() << "canvas is not in backgroundlabel";
    }

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
#endif // QT_DEBUG
