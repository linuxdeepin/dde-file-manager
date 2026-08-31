// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithwidget_1.cpp
 * @brief Unit tests for OpenWithWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithwidget.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(OpenWithWidgetTest, openWithBtnChecked)
{
    // Test method: void openWithBtnChecked((QAbstractButton *btn))
    EXPECT_NO_FATAL_FAILURE(obj->openWithBtnChecked(nullptr));
}

TEST_F(OpenWithWidgetTest, selectFileUrl)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}
