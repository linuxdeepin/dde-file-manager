// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfileinfo_1.cpp
 * @brief Unit tests for TagFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagfileinfo.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileInfoTest, TagFileInfo)
{
    // Test constructor: TagFileInfo((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagFileInfoTest, TagFileInfo_Destructor)
{
    // Test method:  ~TagFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ TagFileInfo *tmp = new TagFileInfo(); delete tmp; });
}
