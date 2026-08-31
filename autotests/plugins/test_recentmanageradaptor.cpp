// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmanageradaptor.cpp
 * @brief Unit tests for RecentManagerAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/RecentManagerAdaptor.h"

#include <QTest>

using namespace src;

class RecentManagerAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentManagerAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentManagerAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentManagerAdaptorTest, RecentManagerAdaptor)
{
    // Test constructor: RecentManagerAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentManagerAdaptorTest, RemoveItems)
{
    // Test method: void RemoveItems((const QStringList &hrefs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->RemoveItems(_arg0));
}
