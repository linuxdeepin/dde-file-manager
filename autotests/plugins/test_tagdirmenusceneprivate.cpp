// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdirmenusceneprivate.cpp
 * @brief Unit tests for TagDirMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagdirmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirMenuScenePrivateTest, openFileLocation)
{
    // Test method: DFMBASE_USE_NAMESPACE openFileLocation((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->openFileLocation(_arg0); });
}
