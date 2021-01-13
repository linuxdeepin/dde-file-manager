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
#include "stub.h"

using namespace testing;
using namespace ScreenSaverCtrlFunction;

TEST(ScreenSaverCtrlTest, needShowScreensaver)
{
    Stub stu;
    static QString str("NN");
    static QStringList strlist;
    strlist << QString("show-screen-saver");
    bool (*myjudge)() = [](){return true;};
    bool (*myfalse)() = [](){return false;};
    QStringList (*mystrlist)() = [](){return strlist;};
    stu.set(ADDR(DGioSettings, keys), mystrlist);
    stu.set(ADDR(DGioSettings, value), myfalse);
    bool ret = needShowScreensaver();
    EXPECT_FALSE(ret);

    int (*mymessage)() = [](){return 0;};
    stu.set(ADDR(QDBusMessage, type), mymessage);
    ret = needShowScreensaver();
    EXPECT_FALSE(ret);


    QString (*mystring)() = [](){return str;};
    stu.set(ADDR(QProcessEnvironment, contains), myjudge);
    stu.set(ADDR(QProcessEnvironment, value), mystring);
    ret = needShowScreensaver();
    EXPECT_EQ(ret, false);
}
