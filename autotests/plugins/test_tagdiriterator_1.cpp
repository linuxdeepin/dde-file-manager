// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdiriterator_1.cpp
 * @brief Unit tests for TagDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagdiriterator.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirIteratorTest, TagDirIterator_Destructor)
{
    // Test method:  ~TagDirIterator(())
    EXPECT_NO_FATAL_FAILURE({ TagDirIterator *tmp = new TagDirIterator(); delete tmp; });
}
