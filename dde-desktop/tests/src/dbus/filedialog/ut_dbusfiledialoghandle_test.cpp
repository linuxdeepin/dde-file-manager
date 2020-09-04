#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "dfiledialoghandle.h"

#include <QFileDialog>

#define private public
#define protected public

#include "dbusfiledialoghandle.h"

using namespace testing;
namespace  {
    class DBusFileDialogHandleTest : public Test
    {
    public:
        DBusFileDialogHandleTest():Test()
        {
        }

        virtual void SetUp() override {
            p_handle = new DBusFileDialogHandle();
        }

        virtual void TearDown() override {
            delete p_handle;
        }

        DBusFileDialogHandle * p_handle;
    };
    class MyDBusFileDialogHandle : public DBusFileDialogHandle
    {
    public:
        MyDBusFileDialogHandle() : DBusFileDialogHandle(){}

        QString directory() const{
            return DFileDialogHandle::directory().absolutePath();
        }
        QString directoryUrl() const
        {
            return DFileDialogHandle::directoryUrl().toString();
        }
        int filter() const
        {
            return DFileDialogHandle::filter();
        }
    };
}

TEST_F(DBusFileDialogHandleTest, get_directory)
{
    QString result = p_handle->directory();

    MyDBusFileDialogHandle myHandle;
    QString expectResult = myHandle.directory();

    EXPECT_EQ(result, expectResult);
}

TEST_F(DBusFileDialogHandleTest, set_directory_url)
{
    MyDBusFileDialogHandle myHandle;
    QString directory("");
    p_handle->setDirectoryUrl(directory);
    QString expectResult = myHandle.directoryUrl();
    EXPECT_FALSE(expectResult.isEmpty());
}

TEST_F(DBusFileDialogHandleTest, get_directory_url)
{
    MyDBusFileDialogHandle myHandle;
    QString directory="tset";
    p_handle->setDirectoryUrl(directory);
    QString expectResult = myHandle.directoryUrl();
    EXPECT_FALSE(expectResult.isEmpty());
}

TEST_F(DBusFileDialogHandleTest, set_select_url)
{
    QString url("");
    p_handle->selectUrl(url);
    p_handle->selectUrl("/home");
    QStringList selecteds = p_handle->selectedUrls();
    EXPECT_TRUE(selecteds.isEmpty());
}

TEST_F(DBusFileDialogHandleTest, get_select_urls)
{
    QStringList selecteds = p_handle->selectedUrls();
    EXPECT_TRUE(selecteds.isEmpty());
}

TEST_F(DBusFileDialogHandleTest, get_filter)
{
    MyDBusFileDialogHandle myHandle;
    int result = p_handle->filter();
    int expectResult = myHandle.filter();
    EXPECT_EQ(result, expectResult);
}

TEST_F(DBusFileDialogHandleTest, set_filter)
{    
    p_handle->setFilter(1);
    EXPECT_EQ(1, p_handle->filter());

    p_handle->setFilter(2);
    EXPECT_EQ(2, p_handle->filter());

    p_handle->setFilter(4);
    EXPECT_EQ(4, p_handle->filter());

    p_handle->setFilter(8);
    EXPECT_EQ(8, p_handle->filter());
}

TEST_F(DBusFileDialogHandleTest, set_view_mode)
{
    QFileDialog::ViewMode mode = QFileDialog::Detail;
    p_handle->setViewMode(mode);

    int result = p_handle->viewMode();
    EXPECT_EQ(mode, result);
}

TEST_F(DBusFileDialogHandleTest, get_view_mode)
{
    QFileDialog::ViewMode mode = QFileDialog::List;
    p_handle->setViewMode(mode);

    int result = p_handle->viewMode();
    EXPECT_EQ(mode, result);
}

TEST_F(DBusFileDialogHandleTest, set_file_mode)
{
    QFileDialog::FileMode mode = QFileDialog::ExistingFiles;
    p_handle->setFileMode(mode);
}

TEST_F(DBusFileDialogHandleTest, set_accept_mode)
{
    QFileDialog::AcceptMode mode = QFileDialog::AcceptOpen;
    p_handle->setAcceptMode(mode);

    int result = p_handle->acceptMode();
    EXPECT_EQ(mode, result);
}

TEST_F(DBusFileDialogHandleTest, get_accept_mode)
{
    QFileDialog::AcceptMode mode = QFileDialog::AcceptSave;
    p_handle->setAcceptMode(mode);

    int result = p_handle->acceptMode();
    EXPECT_EQ(mode, result);
}

TEST_F(DBusFileDialogHandleTest, set_label_test)
{
    QFileDialog::DialogLabel label = QFileDialog::Accept;
    QString txt("test");
    p_handle->setLabelText(label, txt);
    QString result = p_handle->labelText(label);
    EXPECT_EQ(txt, result);
}
TEST_F(DBusFileDialogHandleTest, get_label_test)
{
    QFileDialog::DialogLabel label = QFileDialog::Reject;
    QString txt("test");
    p_handle->setLabelText(label, txt);
    QString result = p_handle->labelText(label);
    EXPECT_EQ(txt, result);
}
TEST_F(DBusFileDialogHandleTest, set_options)
{
    int options = QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails;
    p_handle->setOptions(options);

    int result = p_handle->options();
    EXPECT_EQ(options, result);
}

TEST_F(DBusFileDialogHandleTest, set_option)
{
    int option = QFileDialog::ShowDirsOnly;
    p_handle->setOption(option);

    bool result = p_handle->testOption(option);
    EXPECT_TRUE(result);
}

TEST_F(DBusFileDialogHandleTest, get_options)
{
    int options = QFileDialog::DontUseNativeDialog | QFileDialog::DontUseSheet;
    p_handle->setOptions(options);

    int result = p_handle->options();
    EXPECT_EQ(options, result);
}

TEST_F(DBusFileDialogHandleTest, test_option)
{
    int option = QFileDialog::ReadOnly;
    p_handle->setOption(option);

    bool result = p_handle->testOption(option);
    EXPECT_TRUE(result);
}

TEST_F(DBusFileDialogHandleTest, get_win_id)
{
    WId id = p_handle->winId();
    EXPECT_NE(id, 0);
}

TEST_F(DBusFileDialogHandleTest, set_window_titile)
{
    p_handle->setWindowTitle(QString("test"));
}

TEST_F(DBusFileDialogHandleTest, get_winodw_action)
{
    p_handle->activateWindow();
    p_handle->windowActive();
}

TEST_F(DBusFileDialogHandleTest, set_activate_window)
{
    p_handle->activateWindow();
    p_handle->windowActive();
}

TEST_F(DBusFileDialogHandleTest, heart_beat_interval)
{
    int interval = p_handle->heartbeatInterval();
    EXPECT_EQ(interval, p_handle->m_heartbeatTimer.interval());
}

TEST_F(DBusFileDialogHandleTest, make_heart_beat)
{
    p_handle->makeHeartbeat();
    EXPECT_TRUE(p_handle->m_heartbeatTimer.isActive());
}

TEST_F(DBusFileDialogHandleTest, get_window_flags)
{
    quint32 windowFlags = p_handle->windowFlags() & Qt::Dialog;
    p_handle->setWindowFlags(windowFlags);
    quint32 result = p_handle->windowFlags();
    EXPECT_NE(windowFlags, result);
}

TEST_F(DBusFileDialogHandleTest, set_heartbeat_interval)
{
    int interval = 100;
    p_handle->setHeartbeatInterval(interval);
    EXPECT_EQ(interval, p_handle->m_heartbeatTimer.interval());
}

TEST_F(DBusFileDialogHandleTest, set_window_flags)
{
    quint32 windowFlags = p_handle->windowFlags() & Qt::Desktop;
    p_handle->setWindowFlags(windowFlags);
    quint32 result = p_handle->windowFlags();
    EXPECT_NE(windowFlags, result);
}
