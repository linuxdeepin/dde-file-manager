// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taghelper.cpp
 * @brief Unit tests for TagHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/taghelper.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagHelperTest, commonUrls)
{
    // Test method: QList<QUrl> commonUrls((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->commonUrls(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
