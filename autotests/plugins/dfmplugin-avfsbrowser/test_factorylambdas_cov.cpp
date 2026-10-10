// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_factorylambdas_cov.cpp - SchemeFactory 注册 lambda 补测。
// 生产代码 regClass<T>(scheme) 注册的工厂 lambda 只有 create(scheme)
// 被调用时才执行；本文件用独立测试 scheme 注册后立即 create，
// 断言对象构造成功（create 前置条件：scheme 先注册到 UrlRoute）。

#include <plugins/filemanager/dfmplugin-avfsbrowser/files/avfsfileinfo.h>
#include <plugins/filemanager/dfmplugin-avfsbrowser/files/avfsfilewatcher.h>
#include <plugins/filemanager/dfmplugin-avfsbrowser/files/avfsfileiterator.h>
#include <gtest/gtest.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>

class FactoryLambdaSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};


TEST_F(FactoryLambdaSweepTest, Create_AvfsFileWatcher_AfterRegClass_Constructs)
{
    // Arrange
    const QString scheme = "ut-factory-avfsfilewatcher";
    QString err;
    ASSERT_TRUE(dfmbase::UrlRoute::regScheme(scheme, "/")) << "UrlRoute regScheme failed";

    // Act: register on a private scheme, then create — this executes the
    // regClass factory lambda that constructs the plugin class.
    ASSERT_TRUE(dfmbase::WatcherFactory::regClass<dfmplugin_avfsbrowser::AvfsFileWatcher>(scheme)) << err.toStdString();
    const QUrl url(scheme + ":///sweep-entry");
    auto obj = dfmbase::WatcherFactory::create<dfmplugin_avfsbrowser::AvfsFileWatcher>(url, true, &err);

    // Assert
    EXPECT_FALSE(obj.isNull());
}

TEST_F(FactoryLambdaSweepTest, Create_AvfsFileIterator_AfterRegClass_Constructs)
{
    // Arrange
    const QString scheme = "ut-factory-avfsfileiterator";
    QString err;
    ASSERT_TRUE(dfmbase::UrlRoute::regScheme(scheme, "/")) << "UrlRoute regScheme failed";

    // Act: register on a private scheme, then create — this executes the
    // regClass factory lambda that constructs the plugin class.
    ASSERT_TRUE(dfmbase::DirIteratorFactory::regClass<dfmplugin_avfsbrowser::AvfsFileIterator>(scheme, &err)) << err.toStdString();
    const QUrl url(scheme + ":///sweep-entry");
    auto obj = dfmbase::DirIteratorFactory::create<dfmplugin_avfsbrowser::AvfsFileIterator>(url, &err);

    // Assert
    EXPECT_FALSE(obj.isNull());
}
