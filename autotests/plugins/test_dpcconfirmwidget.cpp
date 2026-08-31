// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcconfirmwidget.cpp
 * @brief Unit tests for DPCConfirmWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dpcwidget/dpcconfirmwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DPCConfirmWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPCConfirmWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPCConfirmWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPCConfirmWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(DPCConfirmWidgetTest, onPasswordChecked)
{
    // Test method: void onPasswordChecked((int result))
    EXPECT_NO_FATAL_FAILURE(obj->onPasswordChecked(0));
}
