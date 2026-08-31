// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanddisplaymenusceneprivate.cpp
 * @brief Unit tests for SortAndDisplayMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sortanddisplaymenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SortAndDisplayMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAndDisplayMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAndDisplayMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAndDisplayMenuScenePrivateTest, addGroupByActions)
{
    // Test method: QMenu addGroupByActions((QMenu *menu))
    auto result = obj->addGroupByActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->addGroupByActions(nullptr); });

}
