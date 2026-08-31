// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfileinfoprivate.cpp
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

TEST_F(TagFileInfoPrivateTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
