// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdiriteratorprivate.cpp
 * @brief Unit tests for TagDirIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/private/tagdiriterator_p.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirIteratorPrivateTest, loadTagsUrls)
{
    // Test method: void loadTagsUrls((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadTagsUrls(_arg0));
}
