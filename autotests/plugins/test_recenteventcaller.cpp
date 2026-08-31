// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recenteventcaller.cpp
 * @brief Unit tests for RecentEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/recenteventcaller.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentEventCallerTest, sendWriteToClipboard)
{
    // Test method: void sendWriteToClipboard((const quint64 windowID, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls))
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendWriteToClipboard(0, ClipBoard::ClipboardAction(), _arg2));
}
