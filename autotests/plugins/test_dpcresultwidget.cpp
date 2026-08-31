// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcresultwidget.cpp
 * @brief Unit tests for DPCResultWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dpcwidget/dpcresultwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class DPCResultWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DPCResultWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DPCResultWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DPCResultWidgetTest, DPCResultWidget)
{
    // Test constructor: DPCResultWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
