// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opendirmenuscene.cpp
 * @brief Unit tests for OpenDirMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/opendirmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenDirMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenDirMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenDirMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenDirMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}
