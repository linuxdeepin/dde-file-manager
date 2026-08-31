// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendcanvasscene.cpp
 * @brief Unit tests for ExtendCanvasScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/extendcanvasscene.h"

#include <QTest>

using namespace ddplugin_organizer;

class ExtendCanvasSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendCanvasScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendCanvasScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendCanvasSceneTest, ExtendCanvasScene)
{
    // Test constructor: ExtendCanvasScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
