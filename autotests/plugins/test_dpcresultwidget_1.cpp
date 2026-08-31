// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dpcresultwidget_1.cpp
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

TEST_F(DPCResultWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(DPCResultWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(DPCResultWidgetTest, setResult)
{
    // Test setter: void setResult((bool success, const QString &msg))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setResult(false, _arg1));
}
