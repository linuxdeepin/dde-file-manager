// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenusceneprivate.cpp
 * @brief Unit tests for OemMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuScenePrivateTest, childActions)
{
    // Test method: QList<QAction *> childActions((QAction *action))
    auto result = obj->childActions(nullptr);
    EXPECT_TRUE(result.isEmpty());

}
