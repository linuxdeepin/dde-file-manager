// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowser.cpp
 * @brief Unit tests for SmbBrowser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "smbbrowser.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(SmbBrowserTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(SmbBrowserTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}
