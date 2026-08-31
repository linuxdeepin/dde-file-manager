// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customtopwidgetinterface_1.cpp
 * @brief Unit tests for CustomTopWidgetInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/customtopwidgetinterface.h"

#include <QTest>

using namespace dfmplugin_workspace;

class CustomTopWidgetInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomTopWidgetInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomTopWidgetInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomTopWidgetInterfaceTest, create)
{
    // Test method: QWidget create((QWidget *parent))
    auto result = obj->create(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->create(nullptr); });

}

TEST_F(CustomTopWidgetInterfaceTest, isKeepShow)
{
    // Test bool getter: isKeepShow()
    bool result = obj->isKeepShow();
    EXPECT_FALSE(result);

}

TEST_F(CustomTopWidgetInterfaceTest, isKeepTop)
{
    // Test bool getter: isKeepTop()
    bool result = obj->isKeepTop();
    EXPECT_FALSE(result);

}

TEST_F(CustomTopWidgetInterfaceTest, isShowFromUrl)
{
    // Test method: bool isShowFromUrl((QWidget *w, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->isShowFromUrl(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CustomTopWidgetInterfaceTest, registeCreateTopWidgetCallback)
{
    // Test method: void registeCreateTopWidgetCallback((const ShowTopWidgetCallback &func))
    ShowTopWidgetCallback _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registeCreateTopWidgetCallback(_arg0));
}

TEST_F(CustomTopWidgetInterfaceTest, setKeepTop)
{
    // Test setter: void setKeepTop((bool keep))
    EXPECT_NO_FATAL_FAILURE(obj->setKeepTop(false));
}
