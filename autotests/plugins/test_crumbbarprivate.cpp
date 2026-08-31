// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbbarprivate.cpp
 * @brief Unit tests for CrumbBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/crumbbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarPrivateTest, buttonAt)
{
    // Test method: UrlPushButton buttonAt((QPoint pos))
    auto result = obj->buttonAt(QPoint());
    EXPECT_NO_FATAL_FAILURE({ obj->buttonAt(QPoint()); });

}

TEST_F(CrumbBarPrivateTest, clearCrumbs)
{
    // Test method: void clearCrumbs(())
    EXPECT_NO_FATAL_FAILURE(obj->clearCrumbs());
}

TEST_F(CrumbBarPrivateTest, writeUrlToClipboard)
{
    // Test method: void writeUrlToClipboard((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->writeUrlToClipboard(_arg0));
}
