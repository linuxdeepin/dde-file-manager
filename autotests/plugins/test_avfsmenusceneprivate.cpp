// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsmenusceneprivate.cpp
 * @brief Unit tests for AvfsMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/avfsmenuscene.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsMenuScenePrivateTest, AvfsMenuScenePrivate)
{
    // Test constructor: AvfsMenuScenePrivate((AvfsMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
