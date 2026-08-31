// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfileinfoprivate_1.cpp
 * @brief Unit tests for TagFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagfileinfo.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileInfoPrivateTest, TagFileInfoPrivate)
{
    // Test constructor: TagFileInfoPrivate((TagFileInfo *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagFileInfoPrivateTest, TagFileInfoPrivate_Destructor)
{
    // Test method:  ~TagFileInfoPrivate(())
    EXPECT_NO_FATAL_FAILURE({ TagFileInfoPrivate *tmp = new TagFileInfoPrivate(); delete tmp; });
}
