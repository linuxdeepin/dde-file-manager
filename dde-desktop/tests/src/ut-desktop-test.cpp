#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define private public
#include "desktop.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "view/canvasviewmanager.h"
#include "screen/screenhelper.h"

#include <QApplication>
#include <QScreen>
#include <QTest>
#include <QTimer>

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
    Desktop desktop;
    char *base = (char *)(desktop.d.data());
    char *wpVar = base + sizeof (void *) * 2;
    void *wallperper1 = (void *)*(long * )(wpVar);
    desktop.preInit();
    desktop.ShowWallpaperChooser(qApp->primaryScreen()->name());
    void *wallperper2 = (void *)*(long * )(wpVar);
    EXPECT_EQ(wallperper1,nullptr);
    ASSERT_NE(wallperper2,nullptr);
    QEventLoop loop;
    QTimer::singleShot(100,&loop,[&loop,wallperper2](){
        Frame *wallperper = (Frame *)wallperper2;
        wallperper->hide();
        loop.exit();
    });
    loop.exec();
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
