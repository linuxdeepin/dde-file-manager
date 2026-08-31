// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareeventhelper.cpp
 * @brief Unit tests for ShareEventHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/shareeventhelper.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareEventHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareEventHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareEventHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareEventHelperTest, blockDelete)
{
    // Test method: bool blockDelete((quint64, const QList<QUrl> &urls, const QUrl &))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->blockDelete(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}
