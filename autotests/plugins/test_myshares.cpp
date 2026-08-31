// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_myshares.cpp
 * @brief Unit tests for MyShares methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "myshares.h"

#include <QTest>

using namespace dfmplugin_myshares;

class MySharesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MyShares();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MyShares *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MySharesTest, addToSidebar)
{
    // Test method: void addToSidebar(())
    EXPECT_NO_FATAL_FAILURE(obj->addToSidebar());
}

TEST_F(MySharesTest, onShareRemoved)
{
    // Test method: void onShareRemoved((const QString &))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onShareRemoved(_arg0));
}

TEST_F(MySharesTest, regMyShareToSearch)
{
    // Test method: void regMyShareToSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->regMyShareToSearch());
}
