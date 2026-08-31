// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareutils.cpp
 * @brief Unit tests for ShareUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/shareutils.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareUtilsTest, convertToLocalUrl)
{
    // Test method: QUrl convertToLocalUrl((const QUrl &shareUrl))
    QUrl _arg0{};
    auto result = obj->convertToLocalUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShareUtilsTest, makeShareUrl)
{
    // Test method: QUrl makeShareUrl((const QString &path))
    QString _arg0{};
    auto result = obj->makeShareUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ShareUtilsTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}
