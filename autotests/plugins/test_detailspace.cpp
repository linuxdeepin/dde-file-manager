// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailspace.cpp
 * @brief Unit tests for DetailSpace methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "detailspace.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailSpaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailSpace();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailSpace *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailSpaceTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(DetailSpaceTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(DetailSpaceTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
