// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/customtopwidgetinterface.h"

#include <QWidget>
#include <QUrl>

using namespace dfmplugin_workspace;

class CustomTopWidgetInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        widget = new CustomTopWidgetInterface();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
        stub.clear();
    }

    CustomTopWidgetInterface *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CustomTopWidgetInterfaceTest, Constructor_SetsDefaultValues)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_FALSE(widget->isKeepShow());
    EXPECT_FALSE(widget->isKeepTop());
}

TEST_F(CustomTopWidgetInterfaceTest, Create_ReturnsValidWidget)
{
    QWidget *parent = new QWidget();
    
    // Set up a callback function first, otherwise create() returns nullptr
    bool callbackCalled = false;
    CreateTopWidgetCallback callback = [&callbackCalled]() -> QWidget* {
        callbackCalled = true;
        return new QWidget();
    };
    widget->registeCreateTopWidgetCallback(callback);
    
    QWidget *result = widget->create(parent);
    
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->parent(), parent);
    EXPECT_TRUE(callbackCalled);
    
    delete parent;
}

TEST_F(CustomTopWidgetInterfaceTest, IsShowFromUrl_ReturnsFalse)
{
    QWidget *testWidget = new QWidget();
    QUrl url("file:///test");
    
    bool result = widget->isShowFromUrl(testWidget, url);
    
    EXPECT_FALSE(result);
    
    delete testWidget;
}

TEST_F(CustomTopWidgetInterfaceTest, SetKeepShow_SetsValue)
{
    widget->setKeepShow(true);
    EXPECT_TRUE(widget->isKeepShow());
    
    widget->setKeepShow(false);
    EXPECT_FALSE(widget->isKeepShow());
}

TEST_F(CustomTopWidgetInterfaceTest, SetKeepTop_SetsValue)
{
    widget->setKeepTop(true);
    EXPECT_TRUE(widget->isKeepTop());
    
    widget->setKeepTop(false);
    EXPECT_FALSE(widget->isKeepTop());
}

TEST_F(CustomTopWidgetInterfaceTest, RegisteCreateTopWidgetCallback_SetsCallback)
{
    bool callbackCalled = false;
    CreateTopWidgetCallback callback = [&callbackCalled]() -> QWidget* {
        callbackCalled = true;
        return new QWidget();
    };
    
    widget->registeCreateTopWidgetCallback(callback);
    
    QWidget *parent = new QWidget();
    QWidget *result = widget->create(parent);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_NE(result, nullptr);
    
    delete parent;
}

TEST_F(CustomTopWidgetInterfaceTest, RegisteShowTopWidgetCallback_SetsCallback)
{
    bool callbackCalled = false;
    ShowTopWidgetCallback callback = [&callbackCalled](QWidget *w, const QUrl &url) -> bool {
        callbackCalled = true;
        return true;
    };
    
    widget->registeCreateTopWidgetCallback(callback);
    
    QWidget *testWidget = new QWidget();
    QUrl url("file:///test");
    
    bool result = widget->isShowFromUrl(testWidget, url);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(result);
    
    delete testWidget;
}
