#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dfilewatcher.h"

#define private public
#define protected public

#include "dbusfiledialogmanager.h"
#include "dbusfiledialoghandle.h"

using namespace testing;
namespace  {
    class DBusFileDialogManagerTest : public Test
    {
    public:
        DBusFileDialogManagerTest():Test()
        {
        }

        virtual void SetUp() override {
            p_manager = new DBusFileDialogManager();
        }

        virtual void TearDown() override {
            delete p_manager;
        }

        DBusFileDialogManager * p_manager;
    };
}

//TEST_F(DBusFileDialogManagerTest, create_dialog)
//{
//    QString key("test");
//    QDBusObjectPath pathObj("/com/deepin/filemanager/filedialog/" + key);

//    QDBusObjectPath result = p_manager->createDialog(key);
//    EXPECT_EQ(pathObj, result);
//    EXPECT_TRUE(p_manager->m_dialogObjectMap.contains(pathObj));
//    p_manager->destroyDialog(result);
//}

//TEST_F(DBusFileDialogManagerTest, destory_dialog)
//{
//    QString key("test");
//    QDBusObjectPath pathObj("/com/deepin/filemanager/filedialog/" + key);

//    QDBusObjectPath result = p_manager->createDialog(key);
//    p_manager->destroyDialog(result);
//    EXPECT_FALSE(p_manager->m_dialogObjectMap.contains(pathObj));
//}

//TEST_F(DBusFileDialogManagerTest, get_all_dialogs)
//{
//    QString key("test");
//    QDBusObjectPath pathObj("/com/deepin/filemanager/filedialog/" + key);

//    p_manager->createDialog(key);

//    QList<QDBusObjectPath> result = p_manager->dialogs();
//    EXPECT_EQ(result, p_manager->m_dialogObjectMap.keys());
//}

TEST_F(DBusFileDialogManagerTest, get_err_string)
{
    QString result = p_manager->errorString();
    EXPECT_EQ(result, p_manager->m_errorString);
}

TEST_F(DBusFileDialogManagerTest, get_use_value)
{
    DFM_USE_NAMESPACE
    bool result = p_manager->isUseFileChooserDialog();
    bool expectResult = DFMApplication::instance()->genericAttribute(DFMApplication::GA_OverrideFileChooserDialog).toBool();
    EXPECT_EQ(result, expectResult);
}

TEST_F(DBusFileDialogManagerTest, get_can_use)
{
    QString testGroup("testGroup");
    QString testFileName("testFileName");
    DFM_USE_NAMESPACE
    bool result = p_manager->canUseFileChooserDialog(testGroup, testFileName);

    const QVariantMap &black_map = DFMApplication::appObtuselySetting()->value("DBusFileDialog", "disable").toMap();
    bool expectResult = !black_map.value(testGroup).toStringList().contains(testFileName);

    EXPECT_EQ(result, expectResult);
}

TEST_F(DBusFileDialogManagerTest, get_glob_mine)
{
    QStringList result = p_manager->globPatternsForMime(QString("text/csv"));
    EXPECT_FALSE(result.isEmpty());

    result = p_manager->globPatternsForMime(QString("abcdefg"));
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DBusFileDialogManagerTest, get_monitor_files)
{
    QStringList result = p_manager->monitorFiles();
    QStringList expectResult = DFileWatcher::getMonitorFiles();
    EXPECT_EQ(result, expectResult);
}

TEST_F(DBusFileDialogManagerTest, show_blue_dialog)
{
    QString id("testID");
    QStringList urls;
    // 显示的窗口测试结束不会自动关闭
//    p_manager->showBluetoothTransDialog(id, urls);
}

//TEST_F(DBusFileDialogManagerTest, handle_destroy_dialog)
//{
//    QString key("test");
//    QDBusObjectPath pathObj("/com/deepin/filemanager/filedialog/" + key);

//    QDBusObjectPath result = p_manager->createDialog(key);
//    p_manager->destroyDialog(pathObj);
//    p_manager->onDialogDestroy();

//    EXPECT_FALSE(p_manager->m_dialogObjectMap.contains(pathObj));
//}
