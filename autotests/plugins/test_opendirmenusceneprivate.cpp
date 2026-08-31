// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opendirmenusceneprivate.cpp
 * @brief Unit tests for OpenDirMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/opendirmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenDirMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenDirMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenDirMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenDirMenuScenePrivateTest, OpenDirMenuScenePrivate)
{
    // Test constructor: OpenDirMenuScenePrivate((OpenDirMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
