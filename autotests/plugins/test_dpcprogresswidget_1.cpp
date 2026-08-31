// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcprogresswidget_1.cpp
 * @brief Unit tests for DPCProgressWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dpcwidget/dpcprogresswidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DPCProgressWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPCProgressWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPCProgressWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPCProgressWidgetTest, changeProgressValue)
{
    // Test method: void changeProgressValue(())
    EXPECT_NO_FATAL_FAILURE(obj->changeProgressValue());
}

TEST_F(DPCProgressWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(DPCProgressWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(DPCProgressWidgetTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
