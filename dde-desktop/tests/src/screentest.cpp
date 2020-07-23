#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <screen/screenmanager.h>
#include <screen/screenobject.h>

#include <screen/screenmanagerwayland.h>
#include <screen/screenobjectwayland.h>
#include <QApplication>
#include <QList>
#include <qpa/qplatformscreen.h>

TEST(screenObj, Screen)
{
    auto screenList = qApp->screens();
    ScreenManager sm;
    auto myScreens = sm.screens();
    ASSERT_EQ(screenList.size(),myScreens.size());

    for (ScreenPointer obj : myScreens){
        bool find = false;
        for (QScreen *sc : screenList){
            if (sc->name() == obj->name()){
                find = true;
                //ASSERT_EQ(sc->availableGeometry(),obj->availableGeometry());
                ASSERT_EQ(sc->handle()->geometry(),obj->handleGeometry());
            }
        }
        ASSERT_EQ(true,find);
    }
}

TEST(screenObjWayland, Screen)
{
    auto screenList = qApp->screens();
    ScreenManagerWayland sm;
    auto myScreens = sm.screens();
    ASSERT_EQ(screenList.size(),myScreens.size());

    for (ScreenPointer obj : myScreens){
        bool find = false;
        for (QScreen *sc : screenList){
            if (sc->name() == obj->name()){
                find = true;
                //ASSERT_EQ(sc->availableGeometry(),obj->availableGeometry());
                ASSERT_EQ(sc->handle()->geometry(),obj->handleGeometry());
            }
        }
        ASSERT_EQ(true,find);
    }
}

TEST(screenObjs, Screen)
{
    ScreenManager sm;
    auto screenList = sm.logicScreens();
    ScreenManagerWayland smw;
    auto myScreens = smw.logicScreens();
    ASSERT_EQ(screenList.size(),myScreens.size());

    for (int i = 0; i < myScreens.size(); ++i){
        ScreenPointer obj = myScreens[i];
        ScreenPointer sc = screenList[i];
        ASSERT_EQ(sc->name(),obj->name());
        ASSERT_EQ(sc->availableGeometry(),obj->availableGeometry());
        ASSERT_EQ(sc->handleGeometry(),obj->handleGeometry());
    }
}

TEST(primaryScreen, Screen){
    ScreenManager sm;
    ASSERT_EQ(qApp->primaryScreen()->name(),sm.primaryScreen()->name());

    ScreenManagerWayland smw;
    ASSERT_EQ(qApp->primaryScreen()->name(),smw.primaryScreen()->name());

    ASSERT_EQ(sm.primaryScreen()->name(),smw.primaryScreen()->name());

    ASSERT_EQ(sm.displayMode(),smw.displayMode());
}
