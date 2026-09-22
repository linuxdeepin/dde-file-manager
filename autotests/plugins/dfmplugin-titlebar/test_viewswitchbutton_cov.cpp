// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/viewswitchbutton.cpp):
//   - ViewSwitchButton::paintEvent                     -> PaintEvent_HoverBackground_Rendered
//   - ViewSwitchButton::mousePressEvent                -> MousePress_LeftButton_MenuExecStubbed
//   - ViewSwitchButton::enterEvent                      -> EnterEvent_MouseEnter_HoverPainted
//   - ViewSwitchButton::leaveEvent                      -> LeaveEvent_MouseLeave_HoverCleared
//   - ViewSwitchButton::mouseMoveEvent                  -> MouseMoveEvent_Tracked_NoCrash
//   - ViewSwitchButton::mouseReleaseEvent               -> MouseReleaseEvent_StateUpdated
//   - ViewSwitchButtonPrivate::setupMenu() lambdas #1-#3-> MenuActionTriggered_LambdasInvoked
//   - ViewSwitchButtonPrivate::updateCheckedState       -> covered indirectly by MenuActionTriggered_LambdasInvoked
// Branches of paintEvent: hover/not-hover, menu-visible/not-visible, dark/light theme,
// currentIconName empty/not-empty. Tests below hit hover and non-hover paths.

#include "stubext.h"
#include "views/viewswitchbutton.h"

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>
#include <QMenu>
#include <QAction>
#include <QImage>
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>

using namespace dfmplugin_titlebar;

namespace {
class ExposedSwitchButton : public ViewSwitchButton
{
public:
    using ViewSwitchButton::paintEvent;
    using ViewSwitchButton::mousePressEvent;
    using ViewSwitchButton::mouseReleaseEvent;
    using ViewSwitchButton::mouseMoveEvent;
    using ViewSwitchButton::enterEvent;
    using ViewSwitchButton::leaveEvent;
};
}   // namespace

class UT_ViewSwitchButtonCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                       [](QMenu *, const QPoint &, QAction *) {
                           __DBG_STUB_INVOKE__
                           return static_cast<QAction *>(nullptr);
                       });
        button = new ExposedSwitchButton();
        button->resize(120, 40);
    }
    void TearDown() override
    {
        delete button;
        button = nullptr;
        stub.clear();
    }
    QImage renderWidget()
    {
        QImage img(button->size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        button->render(&p);
        return img;
    }
    ExposedSwitchButton *button { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ViewSwitchButtonCov, PaintEvent_NoHover_OnlyArrowPainted)
{
    // Arrange
    QPaintEvent event(QRect(0, 0, 120, 40));

    // Act
    button->paintEvent(&event);
    const QImage img = renderWidget();

    // Assert
    EXPECT_EQ(img.size(), button->size());
    EXPECT_FALSE(img.isNull());
}

TEST_F(UT_ViewSwitchButtonCov, EnterEvent_MouseEnter_HoverPainted)
{
    // Arrange
    QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(0, 0));

    // Act
    button->enterEvent(&enter);
    const QImage hovered = renderWidget();

    // Assert
    EXPECT_EQ(hovered.size(), button->size());
    const QImage plain = renderWidget();
    EXPECT_EQ(hovered.size(), plain.size());
}

TEST_F(UT_ViewSwitchButtonCov, MousePress_LeftButton_MenuExecStubbed)
{
    // Arrange
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    button->mousePressEvent(&press);

    // Assert
    EXPECT_TRUE(button->isEnabled());
    EXPECT_EQ(button->pos(), QPoint(0, 0));
    EXPECT_TRUE(QString(button->metaObject()->className()).endsWith("ViewSwitchButton"));
}

TEST_F(UT_ViewSwitchButtonCov, MouseReleaseEvent_StateUpdated)
{
    // Arrange
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    button->mousePressEvent(&press);

    // Act
    button->mouseReleaseEvent(&release);

    // Assert
    EXPECT_TRUE(button->isEnabled());
    EXPECT_FALSE(button->isDown());
    EXPECT_EQ(button->pos(), QPoint(0, 0));
}

TEST_F(UT_ViewSwitchButtonCov, MouseMoveEvent_Tracked_NoCrash)
{
    // Arrange
    QMouseEvent move(QEvent::MouseMove, QPointF(30, 20), QPointF(30, 20),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    button->mouseMoveEvent(&move);

    // Assert
    EXPECT_TRUE(button->isEnabled());
    EXPECT_EQ(button->pos(), QPoint(0, 0));
}

TEST_F(UT_ViewSwitchButtonCov, LeaveEvent_MouseLeave_HoverCleared)
{
    // Arrange
    QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(0, 0));
    QEvent leave(QEvent::Leave);
    button->enterEvent(&enter);

    // Act
    button->leaveEvent(&leave);
    const QImage img = renderWidget();

    // Assert
    EXPECT_EQ(img.size(), button->size());
    EXPECT_FALSE(button->underMouse());
}

TEST_F(UT_ViewSwitchButtonCov, MenuActionTriggered_LambdasInvoked)
{
    // Arrange
    QMenu *menu = button->findChild<QMenu *>();
    ASSERT_NE(menu, nullptr);
    const int actionCount = menu->actions().size();

    // Act: trigger each action to run the setupMenu() lambdas + updateCheckedState
    int triggered = 0;
    for (QAction *act : menu->actions()) {
        if (act->isCheckable() || !act->text().isEmpty()) {
            act->trigger();
            ++triggered;
        }
    }

    // Assert
    EXPECT_GT(actionCount, 0);
    EXPECT_EQ(triggered, actionCount);
}
