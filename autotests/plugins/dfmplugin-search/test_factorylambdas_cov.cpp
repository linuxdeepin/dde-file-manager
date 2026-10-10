// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_factorylambdas_cov.cpp - SchemeFactory 注册 lambda 补测。
// 生产代码 regClass<T>(scheme) 注册的工厂 lambda 只有 create(scheme)
// 被调用时才执行；本文件用独立测试 scheme 注册后立即 create，
// 断言对象构造成功（create 前置条件：scheme 先注册到 UrlRoute）。

#include <plugins/filemanager/dfmplugin-search/fileinfo/searchfileinfo.h>
#include <plugins/filemanager/dfmplugin-search/watcher/searchfilewatcher.h>
#include <plugins/filemanager/dfmplugin-search/iterator/searchdiriterator.h>
#include <gtest/gtest.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>

class FactoryLambdaSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FactoryLambdaSweepTest, Create_SearchFileInfo_AfterRegClass_Constructs)
{
    // Arrange
    const QString scheme = "ut-factory-searchfileinfo";
    QString err;
    ASSERT_TRUE(dfmbase::UrlRoute::regScheme(scheme, "/")) << "UrlRoute regScheme failed";

    // Act: register on a private scheme, then create — this executes the
    // regClass factory lambda that constructs the plugin class.
    ASSERT_TRUE(dfmbase::InfoFactory::regClass<dfmplugin_search::SearchFileInfo>(scheme)) << err.toStdString();
    const QUrl url(scheme + ":///sweep-entry");
    auto obj = dfmbase::InfoFactory::create<dfmplugin_search::SearchFileInfo>(url, dfmbase::Global::CreateFileInfoType::kCreateFileInfoAuto, &err);

    // Assert
    EXPECT_FALSE(obj.isNull());
}

TEST_F(FactoryLambdaSweepTest, Create_SearchFileWatcher_AfterRegClass_Constructs)
{
    // Arrange
    const QString scheme = "ut-factory-searchfilewatcher";
    QString err;
    ASSERT_TRUE(dfmbase::UrlRoute::regScheme(scheme, "/")) << "UrlRoute regScheme failed";

    // Act: register on a private scheme, then create — this executes the
    // regClass factory lambda that constructs the plugin class.
    ASSERT_TRUE(dfmbase::WatcherFactory::regClass<dfmplugin_search::SearchFileWatcher>(scheme)) << err.toStdString();
    const QUrl url(scheme + ":///sweep-entry");
    auto obj = dfmbase::WatcherFactory::create<dfmplugin_search::SearchFileWatcher>(url, true, &err);

    // Assert
    EXPECT_FALSE(obj.isNull());
}

TEST_F(FactoryLambdaSweepTest, Create_SearchDirIterator_AfterRegClass_Constructs)
{
    // Arrange
    const QString scheme = "ut-factory-searchdiriterator";
    QString err;
    ASSERT_TRUE(dfmbase::UrlRoute::regScheme(scheme, "/")) << "UrlRoute regScheme failed";

    // Act: register on a private scheme, then create — this executes the
    // regClass factory lambda that constructs the plugin class.
    ASSERT_TRUE(dfmbase::DirIteratorFactory::regClass<dfmplugin_search::SearchDirIterator>(scheme, &err)) << err.toStdString();
    const QUrl url(scheme + ":///sweep-entry");
    auto obj = dfmbase::DirIteratorFactory::create<dfmplugin_search::SearchDirIterator>(url, &err);

    // Assert
    EXPECT_FALSE(obj.isNull());
}
