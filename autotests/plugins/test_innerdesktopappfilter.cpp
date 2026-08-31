// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_innerdesktopappfilter.cpp
 * @brief Unit tests for InnerDesktopAppFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/filters/innerdesktopappfilter.h"

#include <QTest>

using namespace ddplugin_organizer;

class InnerDesktopAppFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InnerDesktopAppFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InnerDesktopAppFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InnerDesktopAppFilterTest, InnerDesktopAppFilter)
{
    // Test constructor: InnerDesktopAppFilter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(InnerDesktopAppFilterTest, acceptReset)
{
    // Test method: QList<QUrl> acceptReset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->acceptReset(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(InnerDesktopAppFilterTest, changed)
{
    // Test method: void changed((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->changed(_arg0));
}

TEST_F(InnerDesktopAppFilterTest, resetFilter)
{
    // Test method: bool resetFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->resetFilter(_arg0);
    EXPECT_FALSE(result);

}
