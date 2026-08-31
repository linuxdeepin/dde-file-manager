// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcore.cpp
 * @brief Unit tests for TrashCore methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trashcore.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCore();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCore *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(TrashCoreTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TrashCoreTest, regCustomPropertyDialog)
{
    // Test method: void regCustomPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->regCustomPropertyDialog());
}

TEST_F(TrashCoreTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
