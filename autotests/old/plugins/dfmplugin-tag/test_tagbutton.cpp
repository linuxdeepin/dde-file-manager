// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/widgets/tagbutton.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSignalSpy>

using namespace dfmplugin_tag;

class UT_TagButton : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        color = QColor("#ffa503");
        button = new TagButton(color);
    }

    virtual void TearDown() override
    {
        delete button;
        button = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagButton *button = nullptr;
    QColor color;
};

TEST_F(UT_TagButton, constructor)
{
    // Test constructor
    EXPECT_NE(button, nullptr);
    EXPECT_EQ(button->color(), color);
}

TEST_F(UT_TagButton, setRadiusF)
{
    // Test setting radius as float
    EXPECT_NO_THROW(button->setRadiusF(20.0));
}

TEST_F(UT_TagButton, setRadius)
{
    // Test setting radius as size_t
    EXPECT_NO_THROW(button->setRadius(20));
}

TEST_F(UT_TagButton, setCheckable)
{
    // Test setting checkable
    EXPECT_NO_THROW(button->setCheckable(true));
    EXPECT_NO_THROW(button->setCheckable(false));
}

TEST_F(UT_TagButton, setChecked)
{
    // Test setting checked state
    button->setCheckable(true);

    QSignalSpy spy(button, &TagButton::checkedChanged);

    button->setChecked(true);
    EXPECT_TRUE(button->isChecked());
    EXPECT_EQ(spy.count(), 1);

    button->setChecked(false);
    EXPECT_FALSE(button->isChecked());
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(UT_TagButton, setCheckedWhenNotCheckable)
{
    // Test setting checked when not checkable
    button->setCheckable(false);

    button->setChecked(true);
    EXPECT_FALSE(button->isChecked());
}

TEST_F(UT_TagButton, isChecked)
{
    // Test isChecked
    button->setCheckable(true);

    EXPECT_FALSE(button->isChecked());

    button->setChecked(true);
    EXPECT_TRUE(button->isChecked());
}

TEST_F(UT_TagButton, color)
{
    // Test getting color
    QColor buttonColor = button->color();

    EXPECT_EQ(buttonColor, color);
}

TEST_F(UT_TagButton, enterEvent)
{
    // Test enter event
    QSignalSpy spy(button, &TagButton::enter);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QEvent event(QEvent::Enter);
#else
    QEnterEvent event(QPointF(0, 0), QPointF(0, 0), QPointF(0, 0));
#endif

    EXPECT_NO_THROW(button->enterEvent(&event));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagButton, leaveEvent)
{
    // Test leave event
    QSignalSpy spy(button, &TagButton::leave);

    QEvent event(QEvent::Leave);

    EXPECT_NO_THROW(button->leaveEvent(&event));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TagButton, mousePressEvent)
{
    // Test mouse press event
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    EXPECT_NO_THROW(button->mousePressEvent(&event));
}

TEST_F(UT_TagButton, mouseReleaseEvent)
{
    // Test mouse release event
    button->setCheckable(true);

    QSignalSpy clickSpy(button, &TagButton::click);

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    EXPECT_NO_THROW(button->mouseReleaseEvent(&event));
    EXPECT_EQ(clickSpy.count(), 1);
    EXPECT_TRUE(button->isChecked());
}

TEST_F(UT_TagButton, paintEvent)
{
    // Test paint event
    button->setRadius(20);

    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);

    QPaintEvent event(QRect(0, 0, 100, 100));

    // Paint in normal state
    EXPECT_NO_THROW(button->paintEvent(&event));

    // Paint in checked state
    button->setCheckable(true);
    button->setChecked(true);
    EXPECT_NO_THROW(button->paintEvent(&event));
}

TEST_F(UT_TagButton, signals)
{
    // Test all signals
    button->setCheckable(true);
    button->setRadius(20);

    QSignalSpy clickSpy(button, &TagButton::click);
    QSignalSpy enterSpy(button, &TagButton::enter);
    QSignalSpy leaveSpy(button, &TagButton::leave);
    QSignalSpy checkedSpy(button, &TagButton::checkedChanged);

    // Trigger enter
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QEvent enterEvent(QEvent::Enter);
#else
    QEnterEvent enterEvent(QPointF(0, 0), QPointF(0, 0), QPointF(0, 0));
#endif
    button->enterEvent(&enterEvent);
    EXPECT_EQ(enterSpy.count(), 1);

    // Trigger leave
    QEvent leaveEvent(QEvent::Leave);
    button->leaveEvent(&leaveEvent);
    EXPECT_EQ(leaveSpy.count(), 1);

    // Trigger click
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mouseReleaseEvent(&releaseEvent);
    EXPECT_EQ(clickSpy.count(), 1);
    EXPECT_EQ(checkedSpy.count(), 1);
}
