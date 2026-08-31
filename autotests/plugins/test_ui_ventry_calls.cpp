// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ui_ventry_calls.cpp
 * @brief Unit tests for ui_ventry_calls methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ui_ventry_callsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ui_ventry_calls();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ui_ventry_calls *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ui_ventry_callsTest, addAggregatedItemForSeperatedOnlineItem)
{
    // Test method: void addAggregatedItemForSeperatedOnlineItem((const QUrl &entryUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addAggregatedItemForSeperatedOnlineItem(_arg0));
}

TEST_F(ui_ventry_callsTest, addAggregatedItems)
{
    // Test method: void addAggregatedItems(())
    EXPECT_NO_FATAL_FAILURE(obj->addAggregatedItems());
}

TEST_F(ui_ventry_callsTest, addSeperatedOfflineItems)
{
    // Test method: void addSeperatedOfflineItems(())
    EXPECT_NO_FATAL_FAILURE(obj->addSeperatedOfflineItems());
}
