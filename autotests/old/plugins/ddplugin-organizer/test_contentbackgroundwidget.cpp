// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/widgets/contentbackgroundwidget.h"

#include <QPainter>
#include <QPaintEvent>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_ContentBackgroundWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new ContentBackgroundWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ContentBackgroundWidget *widget = nullptr;
};

TEST_F(UT_ContentBackgroundWidget, Constructor_CreatesWidget)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_EQ(widget->parent(), nullptr);
    EXPECT_EQ(widget->radius(), 0);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kNone);
}

TEST_F(UT_ContentBackgroundWidget, Constructor_WithParent_CreatesWidget)
{
    QWidget parent;
    ContentBackgroundWidget child(&parent);
    EXPECT_EQ(child.parent(), &parent);
}

TEST_F(UT_ContentBackgroundWidget, SetRadius_SetsValue)
{
    widget->setRadius(10);
    EXPECT_EQ(widget->radius(), 10);
}

TEST_F(UT_ContentBackgroundWidget, Radius_DefaultValue)
{
    EXPECT_EQ(widget->radius(), 0);
}

TEST_F(UT_ContentBackgroundWidget, SetRoundEdge_SetsValue)
{
    widget->setRoundEdge(ContentBackgroundWidget::kTop);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kTop);
    
    widget->setRoundEdge(ContentBackgroundWidget::kBottom);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kBottom);
    
    widget->setRoundEdge(ContentBackgroundWidget::kBoth);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kBoth);
    
    widget->setRoundEdge(ContentBackgroundWidget::kNone);
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kNone);
}

TEST_F(UT_ContentBackgroundWidget, RoundEdge_DefaultValue)
{
    EXPECT_EQ(widget->roundEdge(), ContentBackgroundWidget::kNone);
}

TEST_F(UT_ContentBackgroundWidget, PaintEvent_DoesNotCrash)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    // Just ensure the paint event doesn't crash
    EXPECT_NO_THROW(widget->paintEvent(&event));
}

TEST_F(UT_ContentBackgroundWidget, Radius_SetterGetter)
{
    for (int radius : {0, 5, 10, 20}) {
        widget->setRadius(radius);
        EXPECT_EQ(widget->radius(), radius);
    }
}

TEST_F(UT_ContentBackgroundWidget, RoundEdge_Values)
{
    EXPECT_NE(ContentBackgroundWidget::kNone, ContentBackgroundWidget::kTop);
    EXPECT_NE(ContentBackgroundWidget::kNone, ContentBackgroundWidget::kBottom);
    EXPECT_NE(ContentBackgroundWidget::kTop, ContentBackgroundWidget::kBottom);
    EXPECT_EQ(ContentBackgroundWidget::kBoth, 
              static_cast<ContentBackgroundWidget::RoundEdge>(
                  ContentBackgroundWidget::kTop | ContentBackgroundWidget::kBottom));
}
