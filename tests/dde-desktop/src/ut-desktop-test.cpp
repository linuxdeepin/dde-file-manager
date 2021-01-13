#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <QScreen>
#include <QTest>
#include <QTimer>

#define private public
#define protected public

#include "desktop.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "view/canvasviewmanager.h"
#include "screen/screenhelper.h"
#include "../stub-ext/stubext.h"
#include "screen/screenmanager.h"

TEST(DesktopTest,init)
{
    Desktop desktop;
    void *dptr = (void *)*(long *)(desktop.d.data());
    desktop.preInit();
    desktop.loadData();
    void *dptr2 = (void *)*(long * )(desktop.d.data());
    EXPECT_EQ(dptr,nullptr);
    EXPECT_NE(dptr2,nullptr);
}

TEST(DesktopTest,load_view)
{
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *viewVar = base + sizeof (void *);
    void *viewManager1 = (void *)*(long * )(viewVar);
    desktop.preInit();
    desktop.loadData();
    desktop.loadView();
    void *viewManager2 = (void *)*(long * )(viewVar);
    EXPECT_EQ(viewManager1,nullptr);
    EXPECT_NE(viewManager2,nullptr);
}

TEST(DesktopTest,show_wallpaper_chooser)
{
    stub_ext::StubExt stu;
    bool isshow = false;
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *wpVar = base + sizeof (void *) * 2;
    void *wallperper1 = (void *)*(long * )(wpVar);
    desktop.preInit();

    stu.set_lamda(ADDR(DBlurEffectWidget, show), [&isshow](){isshow = true; return;});
    desktop.ShowWallpaperChooser(qApp->primaryScreen()->name());
    EXPECT_TRUE(isshow);

    void *wallperper2 = (void *)*(long * )(wpVar);
    EXPECT_EQ(wallperper1,nullptr);
    ASSERT_NE(wallperper2,nullptr);

    qApp->processEvents();
    Frame *wallperper = (Frame *)wallperper2;
    bool ishide = false;
    stu.set_lamda(ADDR(DBlurEffectWidget, hide), [&ishide](){ishide = !ishide; return;});
    wallperper->hide();
    EXPECT_TRUE(ishide);
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
    Desktop desktop;
    desktop.preInit();
    desktop.loadData();
    desktop.loadView();
    char *base = (char *)(desktop.d.data());
    char *viewVar = base + sizeof (void *);
    CanvasViewManager *viewManager = (CanvasViewManager *)*(long * )(viewVar);
    desktop.EnableUIDebug(false);

    QVector<ScreenPointer> screens = ScreenMrg->logicScreens();
    auto canvas = viewManager->canvas();
    for (int i = 0; i < screens.size(); ++i) {
        desktop.SetVisible(i + 1,false);
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
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *viewVar = base + sizeof (void *);
    void *viewManager1 = (void *)*(long * )(viewVar);
    desktop.preInit();
    desktop.loadData();
    desktop.PrintInfo();
    void *viewManager2 = (void *)*(long * )(viewVar);
    EXPECT_EQ(viewManager1, nullptr);

    desktop.loadView();
    desktop.PrintInfo();
    void *viewManager3 = (void *)*(long * )(viewVar);
    EXPECT_NE(viewManager3, nullptr);
}

TEST(DesktopTest, show_wallpaper_setting)
{
    Desktop desktop;
    stub_ext::StubExt stu;
    bool isshow = false;
    char *base = (char *)(desktop.d.data());
    char *view = base + sizeof (void *) * 2;
    void *wset1 = (void *)*(long *)(view);
    EXPECT_EQ(wset1, nullptr);

    stu.set_lamda(ADDR(DBlurEffectWidget, show), [&isshow](){isshow = true; return;});
    desktop.showWallpaperSettings(qApp->primaryScreen()->name(), Frame::Mode::WallpaperMode);
    EXPECT_TRUE(isshow);

    void *wset2 = (void *)*(long *)(view);
    EXPECT_NE(wset2, nullptr);
    Frame* setting = (Frame* )wset2;
    delete setting;
    setting = new Frame(qApp->primaryScreen()->name(), Frame::WallpaperMode);
    desktop.showWallpaperSettings(qApp->primaryScreen()->name(), Frame::Mode::WallpaperMode);

    Frame *wset3 = (Frame *)*(long *)(view);
    emit wset3->done();
    qApp->processEvents();
    void *wset4 = (void *)*(long *)(view);
    EXPECT_EQ(wset4, nullptr);

    desktop.showWallpaperSettings("", Frame::Mode::WallpaperMode);
    bool isset = false;
    stu.set_lamda(VADDR(ScreenManager, primaryScreen), [&isset](){isset = true; return nullptr;});
    desktop.showWallpaperSettings("", Frame::Mode::WallpaperMode);
    delete setting;
}

TEST(DesktopTest, refresh)
{
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *view = base + sizeof (void *);
    void *wset1 = (void *)*(long *)(view);
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();
    desktop.Refresh();

    void *wset2 = (void *)*(long *)(view);
    EXPECT_NE(wset2, nullptr);
}

TEST(DesktopTest, enable_ui_debug)
{
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *view = base + sizeof (void *);
    void *wset1 = (void *)*(long *)(view);
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();
    desktop.EnableUIDebug(true);
    void *wset2 = (void *)*(long *)(view);
    EXPECT_NE(wset2, nullptr);
}

TEST(DesktopTest, get_icon_size)
{
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *view = base + sizeof (void *);
    void *wset1 = (void *)*(long *)(view);
    EXPECT_EQ(wset1, nullptr);

    desktop.preInit();
    desktop.loadView();
    desktop.GetIconSize();

    void *wset2 = (void *)*(long *)(view);
    EXPECT_NE(wset2, nullptr);

    CanvasViewManager* wset3 = (CanvasViewManager *) *(long *)(view);
    QList<int> list;
    if (!wset3->canvas().isEmpty()) {
        list = desktop.GetIconSize();
        EXPECT_EQ(list.at(0), wset3->canvas().first()->iconSize().width());
        EXPECT_EQ(list.at(1), wset3->canvas().first()->iconSize().height());
    }
    else {
        EXPECT_EQ(list.at(0), 0);
        EXPECT_EQ(list.at(1), 0);
    }
}

TEST(DesktopTest, show_Screensaverpaper)
{
    stub_ext::StubExt stu;
    bool isshow = false;
    Desktop* desktop = new Desktop;
    stu.set_lamda(ADDR(DBlurEffectWidget, show), [&isshow](){isshow = true; return;});
    desktop->ShowScreensaverChooser(qApp->primaryScreen()->name());
    EXPECT_TRUE(isshow);
    delete desktop;
}
