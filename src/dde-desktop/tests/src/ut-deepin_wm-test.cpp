#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define private public

#include "../../dde-wallpaper-chooser/dbus/deepin_wm.h"
#include <QDBusConnection>
#include <QObject>
using namespace testing;

namespace  {
       class DeepinWMTest : public Test {
       public:
           DeepinWMTest() : Test() {}

           virtual void SetUp() override {
               deepwm = new DeepinWM("com.deepin.wm", "com/deepin/wm", QDBusConnection::sessionBus());
           }

           virtual void TearDown() override{
               delete deepwm;
           }

           DeepinWM *deepwm = nullptr;
       };
}

TEST_F(DeepinWMTest, static_interfacename)
{
   const char *temp = deepwm->staticInterfaceName();
   EXPECT_EQ("com.deepin.wm", temp);
}

TEST_F(DeepinWMTest, perform_action)
{
    QDBusReply<QString> temp = deepwm->PerformAction(1);
    EXPECT_TRUE(temp.value().isEmpty());
}

TEST_F(DeepinWMTest, set_transientbackground)
{
    QDBusPendingReply<> temp = deepwm->SetTransientBackground("./");
    QDBusMessage mes = temp.reply();
    EXPECT_TRUE(mes.member().isEmpty());
}

TEST_F(DeepinWMTest, cancel_hidewindows)
{
    QDBusReply<QString> temp = deepwm->CancelHideWindows();
    EXPECT_TRUE(temp.value().isEmpty());
}

TEST_F(DeepinWMTest, toggle_debug)
{
    QDBusReply<QString> temp = deepwm->ToggleDebug();
    EXPECT_TRUE(temp.value().isEmpty());
}

TEST_F(DeepinWMTest, request_hide_windows)
{
    QDBusReply<QString> temp = deepwm->RequestHideWindows();
    EXPECT_TRUE(temp.value().isEmpty());
}
