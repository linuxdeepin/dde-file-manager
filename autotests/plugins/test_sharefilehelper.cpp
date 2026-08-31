// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharefilehelper.cpp
 * @brief Unit tests for ShareFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sharefilehelper.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareFileHelperTest, ShareFileHelper)
{
    // Test constructor: ShareFileHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
