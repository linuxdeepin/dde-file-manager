// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_innerdesktopappfilter_1.cpp
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

TEST_F(InnerDesktopAppFilterTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InnerDesktopAppFilterTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(InnerDesktopAppFilterTest, insertFilter)
{
    // Test method: bool insertFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->insertFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InnerDesktopAppFilterTest, refreshModel)
{
    // Test method: void refreshModel(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshModel());
}

TEST_F(InnerDesktopAppFilterTest, refreshModel_refreshM)
{
    // Test method: void refreshModel(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshModel());
}

TEST_F(InnerDesktopAppFilterTest, renameFilter)
{
    // Test method: bool renameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}
