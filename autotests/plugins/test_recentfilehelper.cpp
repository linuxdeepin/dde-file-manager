// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentfilehelper.cpp
 * @brief Unit tests for RecentFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/recentfilehelper.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentFileHelperTest, writeUrlsToClipboard)
{
    // Test method: bool writeUrlsToClipboard((const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls))
    auto result = obj->writeUrlsToClipboard(0, ClipBoard::ClipboardAction(), QList<QUrl>());
    EXPECT_FALSE(result);

}
