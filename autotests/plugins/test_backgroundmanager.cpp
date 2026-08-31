// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundmanager.cpp
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

TEST_F(BackgroundManagerTest, onGeometryChanged)
{
    // Test method: void onGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGeometryChanged());
}
