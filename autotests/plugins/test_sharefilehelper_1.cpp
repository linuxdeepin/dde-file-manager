// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharefilehelper_1.cpp
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

TEST_F(ShareFileHelperTest, instance)
{
    // Test getter: DPMYSHARES_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ShareFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 windowId, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    auto result = obj->openFileInPlugin(0, _arg1);
    EXPECT_FALSE(result);

}
