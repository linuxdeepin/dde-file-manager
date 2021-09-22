#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <dgiofile.h>
#include <dgiofileinfo.h>

#define private public
#include "stub.h"
#include "dfileiconprovider.cpp"
#include "dfileinfo.h"
#include "dgvfsfileinfo.h"
#include "testhelper.h"

using DFM_NAMESPACE::DFileIconProvider;
using DFM_NAMESPACE::DFileIconProviderPrivate;
namespace  {
    class DFileIconProviderTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            provider = new DFileIconProvider;
        }

        virtual void TearDown() override
        {
            delete provider;
            provider = nullptr;
        }

        QString invalidFileName = "/tmp/invalid";
        QString ftpFileName = "ftp://ftp.freebsd.org";
        DFileIconProvider *provider;
    };
}

// 由于下面lambda函数类型转换不能捕获局部变量，故用全局静态变量
static QStringList list {"application-vnd.debian.binary-package",
                    "application-vnd.rar",
                    "application-vnd.ms-htmlhelp",
                    "Zoom.png"
};

/*
TEST_F(DFileIconProviderTest, tst_private)
{
    TestHelper::runInLoop([](){});
    Stub stub;
    QStringList (*themedIconNames)() = []() {
        return QStringList();
    };
    stub.set(ADDR(DGioFileInfo, themedIconNames), themedIconNames);

    QIcon (*fromTheme)(const QString &name) = [](const QString &name) {
        if (std::find(list.begin(), list.end(), name) == list.end()) {
            return QIcon(name);
        } else {
            return QIcon();
        }
    };
    stub.set(static_cast<QIcon (*)(const QString &name)>(&QIcon::fromTheme), fromTheme);

    DFileIconProviderPrivate pri;
    QFileInfo fileinfo;
    fileinfo.setFile("/");
    QIcon icon = pri.getFilesystemIcon(fileinfo);
    EXPECT_FALSE(icon.isNull());

    foreach(const QString var, list) {
        QIcon themeIcon = pri.fromTheme(var);
        EXPECT_FALSE(themeIcon.isNull());
    }
}*/

TEST_F(DFileIconProviderTest, tst_global_instance)
{
    auto instance = DFileIconProvider::globalProvider();
    EXPECT_TRUE(instance != nullptr);
}

/*
TEST_F(DFileIconProviderTest, tst_qicon)
{
    QFileInfo fileinfo;
    fileinfo.setFile("/usr/");
    QIcon icon = provider->icon(fileinfo);
    EXPECT_FALSE(icon.isNull());

    fileinfo.setFile(invalidFileName);
    icon = provider->icon(fileinfo);
    EXPECT_TRUE(icon.isNull());

    Stub stub;
    QIcon (*fromTheme)(const QString &iconName) = [](const QString &) {
        return QIcon();
    };
    stub.set(ADDR(DFileIconProviderPrivate, fromTheme), fromTheme);

    fileinfo.setFile("/usr/");
    icon = provider->icon(fileinfo);
    EXPECT_FALSE(icon.isNull());

    fileinfo.setFile(invalidFileName);
    icon = provider->icon(fileinfo);
    EXPECT_TRUE(icon.isNull());
}*/

#ifndef __arm__
TEST_F(DFileIconProviderTest, tst_dicon)
{
    DFileInfo fileinfo("/home/");
    QIcon icon = provider->icon(fileinfo);
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    DFileInfo fileinfo2(invalidFileName);
    icon = provider->icon(fileinfo2);
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    Stub stub;
    QIcon (*fromTheme)(const QString &iconName) = [](const QString &) {
        return QIcon();
    };
    stub.set(ADDR(DFileIconProviderPrivate, fromTheme), fromTheme);

    icon = provider->icon(fileinfo, QIcon::fromTheme("dialog-warning"));
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    icon = provider->icon(fileinfo2, QIcon::fromTheme("dialog-warning"));
    EXPECT_NO_FATAL_FAILURE(icon.isNull());
}

TEST_F(DFileIconProviderTest, tst_gvfsicon)
{
    const DGvfsFileInfo fileinfo(ftpFileName);
    QIcon icon = provider->icon(fileinfo);
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    const DGvfsFileInfo fileinfo2(invalidFileName);
    icon = provider->icon(fileinfo2);
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    Stub stub;
    QIcon (*fromTheme)(const QString &iconName) = [](const QString &) {
        return QIcon();
    };
    stub.set(ADDR(DFileIconProviderPrivate, fromTheme), fromTheme);

    icon = provider->icon(fileinfo, QIcon::fromTheme("dialog-warning"));
    EXPECT_NO_FATAL_FAILURE(icon.isNull());

    icon = provider->icon(fileinfo2, QIcon::fromTheme("dialog-warning"));
    EXPECT_NO_FATAL_FAILURE(icon.isNull());
}
#endif
