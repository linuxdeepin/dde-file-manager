#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "../../dde-desktop/dbus/dbusdisplay.h"

using namespace testing;

namespace  {
      class DBusPlayTest : public Test {
      public:
          DBusPlayTest() : Test() {

          }

          virtual void SetUp() override  {
              m_dbusdisplay = new DBusDisplay();
          }

          virtual void TearDown() override {
              delete m_dbusdisplay;
          }

          DBusDisplay *m_dbusdisplay = nullptr;
      };
}

TEST_F(DBusPlayTest, static_interface)
{
    EXPECT_EQ("com.deepin.daemon.Display", m_dbusdisplay->staticInterfaceName());
    EXPECT_EQ("/com/deepin/daemon/Display", m_dbusdisplay->staticObjectPath());
    EXPECT_EQ("com.deepin.daemon.Display", m_dbusdisplay->staticServiceName());
}

TEST_F(DBusPlayTest, primary)
{
    QString temp = m_dbusdisplay->primary();
    EXPECT_FALSE(temp.isEmpty());
}

TEST_F(DBusPlayTest, brightness)
{
    BrightnessMap temp = m_dbusdisplay->brightness();
    EXPECT_FALSE(temp.isEmpty());
}

TEST_F(DBusPlayTest, builtin_output)
{
    QDBusObjectPath temp = m_dbusdisplay->builtinOutput();
    EXPECT_TRUE(temp.path().isEmpty());
}

TEST_F(DBusPlayTest, displayMode)
{
    short temp = m_dbusdisplay->displayMode();
    EXPECT_TRUE(temp > 0);
}

TEST_F(DBusPlayTest, has_changed)
{
    bool temp = m_dbusdisplay->hasChanged();
    EXPECT_TRUE(temp == false);
}

TEST_F(DBusPlayTest, monitors)
{
    QList<QDBusObjectPath> temp = m_dbusdisplay->monitors();
    EXPECT_TRUE(temp.size() > 0);
}

TEST_F(DBusPlayTest, primaryrect)
{
    QRect temp = m_dbusdisplay->primaryRect();
    EXPECT_FALSE(temp.isEmpty());
}

TEST_F(DBusPlayTest, screenheight)
{
    ushort temp = m_dbusdisplay->screenHeight();
    EXPECT_TRUE(temp > 0);
}

TEST_F(DBusPlayTest, screenwidth)
{
    ushort temp = m_dbusdisplay->screenWidth();
    EXPECT_TRUE(temp > 0);
}

TEST_F(DBusPlayTest, apply)
{
    QDBusPendingReply<> temp = m_dbusdisplay->Apply();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, associate_touchscreen)
{
    QDBusPendingReply<> temp = m_dbusdisplay->AssociateTouchScreen("ll", "mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, change_brightness)
{
    QDBusPendingReply<> temp = m_dbusdisplay->ChangeBrightness("ll", 32);
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, list_outputnames)
{
    QDBusPendingReply<> temp = m_dbusdisplay->ListOutputNames();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, join_monitor)
{
    QDBusPendingReply<> temp = m_dbusdisplay->JoinMonitor("dd", "mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, query_current_planName)
{
    QDBusPendingReply<> temp = m_dbusdisplay->QueryCurrentPlanName();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, query_outputfeature)
{
    QDBusPendingReply<> temp = m_dbusdisplay->QueryOutputFeature("dd");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, reset)
{
    QDBusPendingReply<> temp = m_dbusdisplay->Reset();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, reset_brightness)
{
    QDBusPendingReply<> temp = m_dbusdisplay->ResetBrightness("mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, reset_changes)
{
    QDBusPendingReply<> temp = m_dbusdisplay->ResetChanges();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, save_changes)
{
    QDBusPendingReply<> temp = m_dbusdisplay->SaveChanges();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, set_brightness)
{
    QDBusPendingReply<> temp = m_dbusdisplay->SetBrightness("mm", 33);
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, set_primary)
{
    QDBusPendingReply<> temp = m_dbusdisplay->SetPrimary("mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, split_monitor)
{
    QDBusPendingReply<> temp = m_dbusdisplay->SplitMonitor("mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, switch_mode)
{
    QDBusPendingReply<> temp = m_dbusdisplay->SwitchMode(123, "mm");
    EXPECT_TRUE(temp.reply().member().isEmpty());
}

TEST_F(DBusPlayTest, get_realdisplaymode)
{
    QDBusPendingReply<> temp = m_dbusdisplay->GetRealDisplayMode();
    EXPECT_TRUE(temp.reply().member().isEmpty());
}
