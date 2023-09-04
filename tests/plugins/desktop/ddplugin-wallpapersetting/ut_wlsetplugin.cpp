// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wlsetplugin.h"
#include "wallpapersettings.h"
#include "settingsdbusinterface.h"
#include "private/autoactivatewindow.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "private/wallpapersettings_p.h"
#include "stubext.h"
#include <gtest/gtest.h>
#include <QWindow>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

DDP_WALLPAERSETTING_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;
using namespace ddplugin_desktop_util;
class UT_WlSetPlugin : public testing::Test
{
protected:
     virtual void SetUp() override
      {
          wls = new  WlSetPlugin;
          wls->initialize();
      }
      virtual void TearDown() override
      {
          delete wls;
          stub.clear();
      }
      WlSetPlugin *wls = nullptr;
      stub_ext::StubExt stub;
};

TEST_F(UT_WlSetPlugin, start)
{
    stub.set_lamda(&EventHandle::init,[](){
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&WlSetPlugin::registerDBus,[](){
        __DBG_STUB_INVOKE__
        return ;
    });

    EXPECT_TRUE(wls->start());
    wls->stop();
}
TEST_F(UT_WlSetPlugin, registerDBus)
{
    bool call = true;
    wls->handle = new EventHandle;
    wls->registerDBus();
    EXPECT_TRUE(call);
}
TEST_F(UT_WlSetPlugin, stop)
{
    wls->stop();
    EXPECT_EQ(wls->handle,nullptr);
}

class UT_EventHandle : public testing::Test
{
protected:
     virtual void SetUp() override
      {
          event = new  EventHandle;

      }
      virtual void TearDown() override
      {
          delete event;
          stub.clear();
      }
      EventHandle *event = nullptr;
      stub_ext::StubExt stub;
};

TEST_F(UT_EventHandle, init)
{
    bool call = true;
    event->init();
    EXPECT_TRUE(call);
}

TEST_F(UT_EventHandle, onQuit)
{
    event->wallpaperSettings = new WallpaperSettings("window");
    stub.set_lamda(&QObject::deleteLater,[](){
        __DBG_STUB_INVOKE__
        return ;
    });
    event->onQuit();
    EXPECT_EQ(event->wallpaperSettings,nullptr);
}

TEST_F(UT_EventHandle, onChanged)
{
    event->wallpaperSettings = new WallpaperSettings("window");
    bool call = false;
    event->onChanged();
    auto fun_type = static_cast<QPair<QString, QString>(WallpaperSettings::*)()const>(&WallpaperSettings::currentWallpaper);
    stub.set_lamda(fun_type,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
        return QPair<QString,QString>("window","window1");
    });

    event->onChanged();
    EXPECT_TRUE(call);
}
class TestNullScreen : public AbstractScreen
{
public:
    virtual QString name() const { return scname; }
    QRect geometry() const { return QRect(0, 0, 200, 200); }
    QRect availableGeometry() const { return QRect(0, 0, 200, 180); }
    QRect handleGeometry() const { return QRect(0, 0, 250, 250); }
    QString scname;
};
TEST_F(UT_EventHandle, show)
{
    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "sc1";
    ScreenPointer sc1(tmp);
    tmp = new TestNullScreen;
    tmp->scname = "sc2";
    ScreenPointer sc2(tmp);

    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1, sc2 };
    int displayMode = kDuplicate;

    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets, &displayMode, &sc1](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_LogicScreens") {
                               return QVariant::fromValue(rets);
                           } else if (t2 == "slot_ScreenProxy_LastChangedMode") {
                               return QVariant::fromValue(displayMode);
                           } else if (t2 == "slot_ScreenProxy_PrimaryScreen") {
                               return QVariant::fromValue(sc1);
                           }
                       }
                       return QVariant();
                   });

    bool quit = false;
    stub.set_lamda(&EventHandle::onQuit,[&quit](){
        __DBG_STUB_INVOKE__
        quit = true;
        return;
    });

    bool change = false;
    stub.set_lamda(&EventHandle::onChanged,[&change](){
        __DBG_STUB_INVOKE__
        change = true;
        return;
    });

    event->wallpaperSettings = new WallpaperSettings("window");

    event->show("window",0);

    event->wallpaperSettings->quit();
    event->wallpaperSettings->backgroundChanged();
    EXPECT_TRUE(quit);
    EXPECT_TRUE(change);
    EXPECT_EQ(event->wallpaperSettings->d->screenName,"sc1");
    EXPECT_FALSE(event->wallpaperSettings->isActiveWindow());

}

TEST_F(UT_EventHandle, hookCanvasRequest)
{

    stub.set_lamda(&EventHandle::wallpaperSetting,[](){
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_TRUE( event->hookCanvasRequest("window"));
}
