// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionswidgetprivate_1.cpp
 * @brief Unit tests for ViewOptionsWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionswidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsWidgetPrivateTest, getStringListByIntList)
{
    // Test method: QList<QString> getStringListByIntList((const QList<int> &intList))
    QList<int> _arg0{};
    auto result = obj->getStringListByIntList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ViewOptionsWidgetPrivateTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}

TEST_F(ViewOptionsWidgetPrivateTest, showSliderTips)
{
    // Test method: void showSliderTips((Dtk::Widget::DSlider *slider, int pos, const QVariantList &valList))
    QVariantList _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->showSliderTips(nullptr, 0, _arg2));
}

TEST_F(ViewOptionsWidgetPrivateTest, switchMode)
{
    // Test method: void switchMode((ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->switchMode(ViewMode()));
}
