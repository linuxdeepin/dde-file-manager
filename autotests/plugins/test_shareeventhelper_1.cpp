// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareeventhelper_1.cpp
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

TEST_F(ShareEventHelperTest, ShareEventHelper)
{
    // Test constructor: ShareEventHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareEventHelperTest, blockMoveToTrash)
{
    // Test method: bool blockMoveToTrash((quint64, const QList<QUrl> &urls, const QUrl &))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->blockMoveToTrash(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(ShareEventHelperTest, blockPaste)
{
    // Test method: bool blockPaste((quint64, const QList<QUrl> &fromUrls, const QUrl &to))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->blockPaste(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(ShareEventHelperTest, containsShareUrl)
{
    // Test method: bool containsShareUrl((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->containsShareUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShareEventHelperTest, hookSendChangeCurrentUrl)
{
    // Test method: bool hookSendChangeCurrentUrl((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->hookSendChangeCurrentUrl(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ShareEventHelperTest, hookSendOpenWindow)
{
    // Test method: bool hookSendOpenWindow((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->hookSendOpenWindow(_arg0);
    EXPECT_FALSE(result);

}
