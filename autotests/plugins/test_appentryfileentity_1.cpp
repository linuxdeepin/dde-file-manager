// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appentryfileentity_1.cpp
 * @brief Unit tests for AppEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/appentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class AppEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppEntryFileEntityTest, description)
{
    // Test getter: QString description()
    auto result = obj->description();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppEntryFileEntityTest, getFormattedExecCommand)
{
    // Test getter: QString getFormattedExecCommand()
    auto result = obj->getFormattedExecCommand();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(AppEntryFileEntityTest, isAccessable)
{
    // Test bool getter: isAccessable()
    bool result = obj->isAccessable();
    EXPECT_FALSE(result);

}

TEST_F(AppEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(AppEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(AppEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}
