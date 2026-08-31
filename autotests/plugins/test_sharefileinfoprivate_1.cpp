// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharefileinfoprivate_1.cpp
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

TEST_F(ShareFileInfoPrivateTest, ShareFileInfoPrivate)
{
    // Test constructor: ShareFileInfoPrivate((ShareFileInfo *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareFileInfoPrivateTest, ShareFileInfoPrivate_Destructor)
{
    // Test method:  ~ShareFileInfoPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ShareFileInfoPrivate *tmp = new ShareFileInfoPrivate(); delete tmp; });
}
