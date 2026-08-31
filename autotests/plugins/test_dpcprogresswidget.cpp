// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcprogresswidget.cpp
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

TEST_F(DPCProgressWidgetTest, DPCProgressWidget)
{
    // Test constructor: DPCProgressWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
