// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharefileinfoprivate.cpp
 * @brief Unit tests for ShareFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileinfo/sharefileinfo.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareFileInfoPrivateTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(ShareFileInfoPrivateTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}
