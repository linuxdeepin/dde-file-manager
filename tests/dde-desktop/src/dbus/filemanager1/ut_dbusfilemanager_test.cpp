#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <QDBusArgument>

#define private public
#define protected public

#include "dbusfilemanager1.h"

using namespace testing;
namespace  {
    class DBusFileManager1Test : public Test
    {
    public:
        DBusFileManager1Test():Test()
        {

        }

        virtual void SetUp() override {
            p_manager = new DBusFileManager1();
        }

        virtual void TearDown() override {
            delete p_manager;
        }

        DBusFileManager1 * p_manager;
    };
}

TEST_F(DBusFileManager1Test, show_folders)
{
    QStringList urls;
    QString startupID;
    // 调用该接口会打开一个文管窗口，且测试结束后不会自动关闭
//    p_manager->ShowFolders(urls, startupID);
}

TEST_F(DBusFileManager1Test, show_item_properties)
{
    QStringList urls;
    QString startupID;
    // 打开属性显示窗口，测试结束后不会自动关闭
    p_manager->ShowItemProperties(urls, startupID);
}

TEST_F(DBusFileManager1Test, show_items)
{
    QStringList urls;
    QString startupID;
    // 打开文管窗口，测试结束后不会自动关闭
//    p_manager->ShowItems(urls, startupID);
}
TEST_F(DBusFileManager1Test, move_to_trash)
{
    QStringList urls;
    urls<<QString("test1")<<QString("test2");
    p_manager->Trash(urls);
}

TEST_F(DBusFileManager1Test, get_monitor_files)
{
    QStringList values = p_manager->GetMonitorFiles();
    EXPECT_FALSE(values.isEmpty());
}

TEST_F(DBusFileManager1Test, set_task_top)
{
    bool value = p_manager->topTaskDialog();
    EXPECT_FALSE(value);
}

TEST_F(DBusFileManager1Test, close_task)
{
    p_manager->closeTask();
}

TEST_F(DBusFileManager1Test, lock_property_changed)
{
    QObject::connect(p_manager, &DBusFileManager1::lockEventTriggered, [=]{
       EXPECT_TRUE(true);
    });
    QDBusMessage msg;
    QList<QVariant> args;
    args<<QString("com.deepin.SessionManager");
    QDBusArgument argument;
    args<<QVariant::fromValue(argument);
    args<<QString("none");
    msg.setArguments(args);

    p_manager->lockPropertyChanged(msg);
}
