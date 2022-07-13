/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <QScreen>
#include <QTest>
#include <QTimer>
#include <QObject>

#define private public
#define protected public

#include "desktop.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "view/canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "../stub-ext/stubext.h"
#include "screen/screenmanager.h"
#include "../desktopprivate.h"
#include "presenter/apppresenter.h"

TEST(DesktopTest,init)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;
    BackgroundManager* bmanager1 = desktop.d.data()->m_background;
    EXPECT_EQ(bmanager1, nullptr);

    desktop.preInit();
    desktop.loadData();
    BackgroundManager* bmanager2 = desktop.d.data()->m_background;
    EXPECT_NE(bmanager2, nullptr);
}

TEST(DesktopTest,load_view)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;
    CanvasViewManager *viewManager1 = desktop.d.data()->m_canvas;
    EXPECT_EQ(viewManager1,nullptr);

    desktop.preInit();
    desktop.loadData();
    desktop.loadView();
    CanvasViewManager *viewManager2 = desktop.d.data()->m_canvas;
    EXPECT_NE(viewManager2,nullptr);
}

TEST(DesktopTest,show_wallpaper_chooser)
{
    stub_ext::StubExt stu;
    Desktop desktop;
    bool isshow = false;
    stu.set_lamda(ADDR(Desktop, showWallpaperSettings), [&isshow](){isshow = true;});
    desktop.ShowWallpaperChooser(qApp->primaryScreen()->name());
    EXPECT_TRUE(isshow);
}

#ifndef DISABLE_ZONE
#include "../dde-zone/mainwindow.h"
TEST(DesktopTest,show_zone_setting)
{
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *zoneVar = base + sizeof (void *) * 3;
    void *zone1 = (void *)*(long * )(zoneVar);
    desktop.showZoneSettings();
    void *zone2 = (void *)*(long * )(zoneVar);
    EXPECT_EQ(zone1,nullptr);
    ASSERT_NE(zone2,nullptr);
    QEventLoop loop;
    QTimer::singleShot(100,&loop,[&loop,zone2](){
        ZoneMainWindow *zone = (ZoneMainWindow *)zone2;
        zone->hide();
        loop.exit();
    });
    loop.exec();
}
#endif

TEST(DesktopTest,set_visible)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;
    desktop.preInit();
    desktop.loadData();
    desktop.loadView();
    CanvasViewManager *viewManager = desktop.d.data()->m_canvas;
    desktop.EnableUIDebug(false);

    QVector<ScreenPointer> screens = ScreenMrg->logicScreens();
    auto canvas = viewManager->canvas();
    for (int i = 0; i < screens.size(); ++i) {
        desktop.SetVisible(i + 1,false);
        if (!canvas.contains(screens.at(i))) continue;
        CanvasViewPointer view = canvas.value(screens.at(i));
        EXPECT_EQ(false,view->isVisible());

        desktop.FixGeometry(i + 1);
        desktop.SetVisible(i + 1,true);
        EXPECT_EQ(true,view->isVisible());
    }
    desktop.Reset();
}

TEST(DesktopTest, print_info)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;

    CanvasViewManager *viewManager1 = desktop.d.data()->m_canvas;
    desktop.preInit();
    desktop.loadData();
    desktop.PrintInfo();
    EXPECT_EQ(viewManager1, nullptr);

    desktop.loadView();

    auto currentScreens = ScreenMrg->logicScreens();
    for (const ScreenPointer &sp : currentScreens){
        if (desktop.d.data()->m_canvas) {
            CanvasViewPointer mView = CanvasViewPointer(new CanvasGridView(sp->name()));
            desktop.d.data()->m_canvas->m_canvasMap.insert(sp, mView);
        }
    }

    desktop.PrintInfo();
    CanvasViewManager *viewManager3 = desktop.d.data()->m_canvas;

    EXPECT_NE(viewManager3, nullptr);
}

TEST(DesktopTest, show_wallpaper_setting)
{
    Desktop desktop;
    stub_ext::StubExt stu;
    bool autoAct = false;
    stu.set_lamda(&AutoActivateWindow::start,[&autoAct](){autoAct = true;return true;});

    WallpaperSettings *wset1 = desktop.d.data()->wallpaperSettings;
    EXPECT_EQ(wset1, nullptr);

    bool isshow = false;
    stu.set_lamda(ADDR(WallpaperSettings, show), [&isshow](){isshow = true; return;});
    desktop.showWallpaperSettings(qApp->primaryScreen()->name(), Frame::Mode::WallpaperMode);
    EXPECT_TRUE(isshow);
    EXPECT_TRUE(autoAct);

    WallpaperSettings *wset2 = desktop.d.data()->wallpaperSettings;
    EXPECT_NE(wset2, nullptr);
    desktop.showWallpaperSettings(qApp->primaryScreen()->name(), Frame::Mode::WallpaperMode);

    WallpaperSettings *wset3 = desktop.d.data()->wallpaperSettings;
    if (nullptr != wset3)
        emit wset3->done();

    desktop.showWallpaperSettings(qApp->primaryScreen()->name(), Frame::Mode::WallpaperMode);

    WallpaperSettings *wset11 = desktop.d.data()->wallpaperSettings;
    if (wset11 && desktop.d.data()->m_background) {
        bool callDesktopBackground = false;
        bool callSetBackgroundImage = false;
        stu.set_lamda(ADDR(Frame, desktopBackground), [&callDesktopBackground](){
            callDesktopBackground = true;
            return QPair<QString, QString>();
        });
        stu.set_lamda(ADDR(BackgroundManager, setBackgroundImage), [&callSetBackgroundImage](){
            callSetBackgroundImage = true;
            return;
        });
        emit wset11->backgroundChanged();

        EXPECT_TRUE(callDesktopBackground);
        EXPECT_TRUE(callSetBackgroundImage);
    }

    desktop.showWallpaperSettings("", Frame::Mode::WallpaperMode);
    bool isset = false;
    stu.set_lamda(VADDR(ScreenManager, primaryScreen), [&isset](){isset = true; return nullptr;});
    desktop.showWallpaperSettings("", Frame::Mode::WallpaperMode);
}

TEST(DesktopTest, refresh)
{
    stub_ext::StubExt stu;
    bool isrefresh = false;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});
    stu.set_lamda(ADDR(CanvasGridView, Refresh), [&isrefresh](){isrefresh = true;return;});

    Desktop desktop;
    WallpaperSettings *wset1 = desktop.d.data()->wallpaperSettings;
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();
    desktop.Refresh();
    if (desktop.d.data()->m_canvas->canvas().values().size())
        EXPECT_TRUE(isrefresh);
}

TEST(DesktopTest, enable_ui_debug)
{
    stub_ext::StubExt stu;
    bool isDebug = false;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});
    stu.set_lamda(ADDR(CanvasGridView, EnableUIDebug), [&isDebug](){isDebug = true;return;});

    Desktop desktop;
    WallpaperSettings *wset1 = desktop.d.data()->wallpaperSettings;
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();

    auto currentScreens = ScreenMrg->logicScreens();
    for (const ScreenPointer &sp : currentScreens){
        if (desktop.d.data()->m_canvas) {
            CanvasViewPointer mView = CanvasViewPointer(new CanvasGridView(sp->name()));
            desktop.d.data()->m_canvas->m_canvasMap.insert(sp, mView);
        }
    }
    desktop.EnableUIDebug(true);
    if (desktop.d.data()->m_canvas->canvas().values().size())
        EXPECT_TRUE(isDebug);
}

TEST(DesktopTest, get_icon_size)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;
    CanvasViewManager *wset1 = desktop.d.data()->m_canvas;
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();
    desktop.GetIconSize();
}

TEST(DesktopTest, show_Screensaverpaper)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});
    bool autoAct = false;
    stu.set_lamda(&AutoActivateWindow::start,[&autoAct](){autoAct = true;return true;});

    bool isshow = false;
    Desktop* desktop = new Desktop;
    stu.set_lamda(ADDR(WallpaperSettings, show), [&isshow](){isshow = true; return;});
    desktop->ShowScreensaverChooser(qApp->primaryScreen()->name());
    EXPECT_TRUE(isshow);
    EXPECT_TRUE(autoAct);
    delete desktop;
}

TEST(DesktopTest, Fix_Geometry)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(Presenter, init), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, init), [](){return;});

    Desktop desktop;
    QVector<ScreenPointer> screens = ScreenMrg->logicScreens();
    if (!screens.size())
        return;
    //此处会发送一个信号
    desktop.FixGeometry(1);
}
