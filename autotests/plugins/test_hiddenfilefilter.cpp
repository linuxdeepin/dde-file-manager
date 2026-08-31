// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hiddenfilefilter.cpp
 * @brief Unit tests for HiddenFileFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/filters/hiddenfilefilter.h"

#include <QTest>

using namespace ddplugin_organizer;

class HiddenFileFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HiddenFileFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HiddenFileFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HiddenFileFilterTest, acceptReset)
{
    // Test method: QList<QUrl> acceptReset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->acceptReset(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(HiddenFileFilterTest, resetFilter)
{
    // Test method: bool resetFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->resetFilter(_arg0);
    EXPECT_FALSE(result);

}
