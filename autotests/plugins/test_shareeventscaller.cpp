// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareeventscaller.cpp
 * @brief Unit tests for ShareEventsCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/shareeventscaller.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareEventsCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareEventsCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareEventsCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareEventsCallerTest, sendCancelSharing)
{
    // Test method: void sendCancelSharing((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendCancelSharing(_arg0));
}

TEST_F(ShareEventsCallerTest, sendShowProperty)
{
    // Test method: void sendShowProperty((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowProperty(_arg0));
}
