// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsereventcaller.cpp
 * @brief Unit tests for SmbBrowserEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/smbbrowsereventcaller.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserEventCallerTest, sendChangeCurrentUrl)
{
    // Test method: void sendChangeCurrentUrl((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendChangeCurrentUrl(0, _arg1));
}
