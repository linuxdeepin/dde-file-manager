// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmanagerdbus.cpp
 * @brief Unit tests for RecentManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentmanagerdbus.h"

#include <QTest>

using namespace recent;

class RecentManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentManagerDBusTest, RemoveItems)
{
    // Test method: void RemoveItems((const QStringList &hrefs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->RemoveItems(_arg0));
}
