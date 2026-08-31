// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfileiconprovider.cpp
 * @brief Unit tests for LocalFileIconProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localfileiconprovider.h"

#include <QTest>

using namespace src;

class LocalFileIconProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalFileIconProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalFileIconProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalFileIconProviderTest, globalProvider)
{
    // Test getter: LocalFileIconProvider globalProvider()
    auto result = obj->globalProvider();
    EXPECT_NO_FATAL_FAILURE({ obj->globalProvider(); });

}

TEST_F(LocalFileIconProviderTest, icon)
{
    // Test method: QIcon icon((FileInfoPointer info, const QIcon &feedback))
    QIcon _arg1{};
    auto result = obj->icon(FileInfoPointer(), _arg1);
    EXPECT_TRUE(result.isNull());

}
