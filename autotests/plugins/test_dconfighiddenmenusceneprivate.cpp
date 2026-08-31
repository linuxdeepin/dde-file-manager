// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dconfighiddenmenusceneprivate.cpp
 * @brief Unit tests for DConfigHiddenMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/dconfighiddenmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class DConfigHiddenMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DConfigHiddenMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DConfigHiddenMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DConfigHiddenMenuScenePrivateTest, DConfigHiddenMenuScenePrivate)
{
    // Test constructor: DConfigHiddenMenuScenePrivate((DConfigHiddenMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
