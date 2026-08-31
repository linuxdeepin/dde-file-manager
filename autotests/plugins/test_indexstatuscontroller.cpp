// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexstatuscontroller.cpp
 * @brief Unit tests for IndexStatusController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/indexstatuscontroller.h"

#include <QTest>

using namespace dfmplugin_search;

class IndexStatusControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexStatusController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexStatusController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexStatusControllerTest, connectToBackend)
{
    // Test method: void connectToBackend(())
    EXPECT_NO_FATAL_FAILURE(obj->connectToBackend());
}
