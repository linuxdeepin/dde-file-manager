// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundmanager_1.cpp
 * @brief Unit tests for BackgroundManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundmanager.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundManagerTest, BackgroundManager)
{
    // Test constructor: BackgroundManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BackgroundManagerTest, allBackgroundPath)
{
    // Test getter: QMap<QString, QString> allBackgroundPath()
    auto result = obj->allBackgroundPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundManagerTest, allBackgroundWidgets)
{
    // Test getter: QMap<QString, BackgroundWidgetPointer> allBackgroundWidgets()
    auto result = obj->allBackgroundWidgets();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundManagerTest, backgroundPath)
{
    // Test method: QString backgroundPath((const QString &screen))
    QString _arg0{};
    auto result = obj->backgroundPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundManagerTest, backgroundWidget)
{
    // Test method: BackgroundWidgetPointer backgroundWidget((const QString &screen))
    QString _arg0{};
    auto result = obj->backgroundWidget(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(BackgroundManagerTest, createBackgroundWidget)
{
    // Test method: BackgroundWidgetPointer createBackgroundWidget((QWidget *root))
    auto result = obj->createBackgroundWidget(nullptr);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(BackgroundManagerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(BackgroundManagerTest, onBackgroundBuild)
{
    // Test method: void onBackgroundBuild(())
    EXPECT_NO_FATAL_FAILURE(obj->onBackgroundBuild());
}

TEST_F(BackgroundManagerTest, onBackgroundChanged)
{
    // Test method: void onBackgroundChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onBackgroundChanged());
}

TEST_F(BackgroundManagerTest, onConfigChanged)
{
    // Test method: void onConfigChanged((const QString &cfg, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onConfigChanged(_arg0, _arg1));
}

TEST_F(BackgroundManagerTest, onDetachWindows)
{
    // Test method: void onDetachWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->onDetachWindows());
}

TEST_F(BackgroundManagerTest, restBackgroundManager)
{
    // Test method: void restBackgroundManager(())
    EXPECT_NO_FATAL_FAILURE(obj->restBackgroundManager());
}

TEST_F(BackgroundManagerTest, useColorBackground)
{
    // Test bool getter: useColorBackground()
    bool result = obj->useColorBackground();
    EXPECT_FALSE(result);

}

TEST_F(BackgroundManagerTest, BackgroundManager_Destructor)
{
    // Test method:  ~BackgroundManager(())
    EXPECT_NO_FATAL_FAILURE({ BackgroundManager *tmp = new BackgroundManager(); delete tmp; });
}
