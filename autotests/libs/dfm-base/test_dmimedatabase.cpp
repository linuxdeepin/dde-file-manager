// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dmimedatabase.cpp
 * @brief Unit tests for DMimeDatabase methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include <dfm-base/mimetype/dmimedatabase.h>

#include <QTest>
#include <QUrl>
#include <QMimeDatabase>

using namespace dfmbase;

class DMimeDatabaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DMimeDatabase();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DMimeDatabase *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DMimeDatabaseTest, DMimeDatabase)
{
    ASSERT_NE(obj, nullptr);
}

TEST_F(DMimeDatabaseTest, mimeTypeForUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    auto result = obj->mimeTypeForUrl(url);
    EXPECT_TRUE(result.isValid());
}
