#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <dgiosettings.h>
#include <QString>
#include <QProcessEnvironment>
#include "../dde-wallpaper-chooser/screensavercontrol.h"
#include "stubext.h"
using namespace testing;
using namespace ScreenSaverCtrlFunction;

TEST(ScreenSaverCtrlTest, needShowScreensaver)
{
    stub_ext::StubExt stu;
    QString str("NN");
    QStringList strlist;
    strlist << QString("show-screen-saver");
    stu.set_lamda(ADDR(DGioSettings, keys), [&strlist](){return strlist;});
    stu.set_lamda(ADDR(DGioSettings, value), [](){return false;});
    bool ret = needShowScreensaver();
    EXPECT_FALSE(ret);

    stu.set_lamda(ADDR(QDBusMessage, type), [](){return QDBusMessage::MessageType::InvalidMessage;});
    ret = needShowScreensaver();
    EXPECT_FALSE(ret);

    stu.set_lamda(ADDR(QProcessEnvironment, contains),[](){return true;});
    stu.set_lamda(ADDR(QProcessEnvironment, value), [&str](){return str;});
    ret = needShowScreensaver();
    EXPECT_EQ(ret, false);
}
