// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/widgets/entrywidget.h"

#include <QWidget>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_EntryWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test objects
        leftWidget = new QWidget();
        rightWidget = new QWidget();
        parent = new QWidget();
    }

    void TearDown() override
    {
        // delete leftWidget;
        // delete rightWidget;
        // delete parent;
        if (widget) {
            delete widget;
            widget = nullptr;
        }
        stub.clear();
    }

public:
    QWidget *leftWidget = nullptr;
    QWidget *rightWidget = nullptr;
    QWidget *parent = nullptr;
    EntryWidget *widget = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_EntryWidget, Constructor_CreatesWidget)
{
    widget = new EntryWidget(leftWidget, rightWidget);
    
    EXPECT_NE(&widget, nullptr);
    EXPECT_EQ(widget->parent(), nullptr);
    EXPECT_EQ(widget->widget(true), leftWidget);
    EXPECT_EQ(widget->widget(false), rightWidget);
}

TEST_F(UT_EntryWidget, Constructor_WithParent_CreatesWidget)
{
    widget = new EntryWidget(leftWidget, rightWidget, parent);
    
    EXPECT_EQ(widget->parent(), parent);
    EXPECT_EQ(widget->widget(true), leftWidget);
    EXPECT_EQ(widget->widget(false), rightWidget);
}

TEST_F(UT_EntryWidget, Widget_OnlyLeftWidget)
{
    widget = new EntryWidget(leftWidget, nullptr);
    
    EXPECT_EQ(widget->widget(true), leftWidget);
    EXPECT_EQ(widget->widget(false), nullptr);
}
TEST_F(UT_EntryWidget, Widget_OnlyRightWidget)
{
    widget = new EntryWidget(nullptr, rightWidget);
    
    EXPECT_EQ(widget->widget(true), nullptr);
    EXPECT_EQ(widget->widget(false), rightWidget);
}

TEST_F(UT_EntryWidget, Inheritance_FromContentBackgroundWidget)
{
    widget = new EntryWidget(leftWidget, rightWidget);
    
    // Test that it inherits from ContentBackgroundWidget
    ContentBackgroundWidget *basePtr = dynamic_cast<ContentBackgroundWidget *>(widget);
    EXPECT_NE(basePtr, nullptr);
    
    // Test inherited methods
    EXPECT_EQ(widget->radius(), 0);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kNone);
    
    widget->setRadius(10);
    EXPECT_EQ(widget->radius(), 10);
}
