// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanddisplaymenuscene.cpp
 * @brief Unit tests for SortAndDisplayMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sortanddisplaymenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SortAndDisplayMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAndDisplayMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAndDisplayMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAndDisplayMenuSceneTest, SortAndDisplayMenuScene)
{
    // Test constructor: SortAndDisplayMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
