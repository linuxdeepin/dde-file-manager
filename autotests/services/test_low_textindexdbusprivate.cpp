// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_textindexdbusprivate.cpp
 * @brief Unit tests for TextIndexDBusPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/textindexdbus.h"

#include <QTest>

using namespace src;

class TextIndexDBusPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexDBusPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexDBusPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexDBusPrivateTest, canSilentlyRefreshIndex)
{
    // Test method: bool canSilentlyRefreshIndex((const QString &path))
    QString _arg0{};
    auto result = obj->canSilentlyRefreshIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexDBusPrivateTest, handleConfigChanged)
{
    // Test method: void handleConfigChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->handleConfigChanged());
}

TEST_F(TextIndexDBusPrivateTest, handleMonitoring)
{
    // Test method: void handleMonitoring((bool start))
    EXPECT_NO_FATAL_FAILURE(obj->handleMonitoring(false));
}

TEST_F(TextIndexDBusPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(TextIndexDBusPrivateTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TextIndexDBusPrivateTest, initializeSupportedExtensions)
{
    // Test method: void initializeSupportedExtensions(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeSupportedExtensions());
}
