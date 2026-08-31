// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfilehelper.cpp
 * @brief Unit tests for TagFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/tagfilehelper.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 windowId, const QList<QUrl> urls))
    auto result = obj->openFileInPlugin(0, QList<QUrl>());
    EXPECT_FALSE(result);

}
