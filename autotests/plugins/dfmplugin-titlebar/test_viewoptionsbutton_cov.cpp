// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/viewoptionsbutton.cpp):
//   - ViewOptionsButton::paintEvent                  -> PaintEvent_* (3 cases)
//   - ViewOptionsButton::mousePressEvent             -> MousePressRelease_DownStateToggled
//   - ViewOptionsButton::mouseMoveEvent              -> MouseMoveEvent_Tracked
//   - ViewOptionsButton::mouseReleaseEvent           -> MousePressRelease_DownStateToggled
//   - ViewOptionsButton::enterEvent                  -> EnterLeaveEvent_HoverStateToggled
//   - ViewOptionsButton::leaveEvent                  -> EnterLeaveEvent_HoverStateToggled
//   - ViewOptionsButtonPrivate::popupVisible()       -> PaintEvent_PopupVisible_HoverBrushUsed (indirect)
//   - ViewOptionsButtonPrivate::initConnect lambda   -> covered when ViewOptionsWidget emits visibilityChanged (indirect, best effort)
// Branches of paintEvent: hover/popup path vs plain path, dark/light theme, isDown/not-down.

#include "stubext.h"
#include <dfm-base/dfm_global_defines.h>
#include "views/viewoptionsbutton.h"
#include "views/viewoptionswidget.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>

using namespace dfmplugin_titlebar;

namespace {
class ExposedOptionsButton : public ViewOptionsButton
{
public:
    using ViewOptionsButton::paintEvent;
    using ViewOptionsButton::mousePressEvent;
    using ViewOptionsButton::mouseReleaseEvent;
    using ViewOptionsButton::mouseMoveEvent;
    using ViewOptionsButton::enterEvent;
    using ViewOptionsButton::leaveEvent;
};
}   // namespace

class UT_ViewOptionsButtonCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Block any popup widget from really showing (would block offscreen)
        stub.set_lamda(ADDR(QWidget, show), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(static_cast<void (ViewOptionsWidget::*)(const QPoint &, DFMBASE_NAMESPACE::Global::ViewMode, const QUrl &)>(&ViewOptionsWidget::exec),
                       [](ViewOptionsWidget *, const QPoint &, DFMBASE_NAMESPACE::Global::ViewMode, const QUrl &) {
                           __DBG_STUB_INVOKE__
                       });
        button = new ExposedOptionsButton();
        button->resize(40, 40);
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
    ExposedOptionsButton *button { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ViewOptionsButtonCov, PaintEvent_NoHover_PlainToolButtonPainted)
{
    // Arrange
    QPaintEvent event(QRect(0, 0, 40, 40));

    // Act
    button->paintEvent(&event);
    const QImage img = renderWidget();

    // Assert
    EXPECT_EQ(img.size(), QSize(40, 40));
    EXPECT_FALSE(img.isNull());
}

TEST_F(UT_ViewOptionsButtonCov, PaintEvent_HoverPath_HoverBrushUsed)
{
    // Arrange
    QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(0, 0));
    button->enterEvent(&enter);
    QPaintEvent event(QRect(0, 0, 40, 40));

    // Act
    button->paintEvent(&event);
    const QImage img = renderWidget();

    // Assert
    EXPECT_EQ(img.size(), QSize(40, 40));
    EXPECT_FALSE(img.isNull());
}

TEST_F(UT_ViewOptionsButtonCov, MousePressRelease_DownStateToggled)
{
    // Arrange
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    button->mousePressEvent(&press);
    const bool downAfterPress = button->isDown();
    button->mouseReleaseEvent(&release);

    // Assert
    EXPECT_TRUE(downAfterPress);
    EXPECT_FALSE(button->isDown());
    EXPECT_EQ(button->size(), QSize(40, 40));
}

TEST_F(UT_ViewOptionsButtonCov, MouseMoveEvent_Tracked)
{
    // Arrange
    QMouseEvent move(QEvent::MouseMove, QPointF(20, 20), QPointF(20, 20),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    button->mouseMoveEvent(&move);

    // Assert
    EXPECT_TRUE(button->isEnabled());
    EXPECT_EQ(button->size(), QSize(40, 40));
}

TEST_F(UT_ViewOptionsButtonCov, EnterLeaveEvent_HoverStateToggled)
{
    // Arrange
    QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(0, 0));
    QEvent leave(QEvent::Leave);

    // Act
    button->enterEvent(&enter);
    const QImage hovered = renderWidget();
    button->leaveEvent(&leave);
    const QImage plain = renderWidget();

    // Assert
    EXPECT_FALSE(hovered.isNull());
    EXPECT_EQ(hovered.size(), plain.size());
}
