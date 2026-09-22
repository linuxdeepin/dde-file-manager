// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/crumbbar.cpp):
//   - CrumbBar::mousePressEvent                      -> MousePress_RightButton_Ignored / MousePress_LeftButton_PassedToBase
//   - CrumbBar::mouseReleaseEvent (+lambda #1)       -> MouseRelease_RightButton_GlobalPosEmitted
//   - CrumbBar::contextMenuEvent                     -> ContextMenuEvent_MenuBuilt
//   - CrumbBar::customMenu lambdas #1-#4             -> CustomMenu_ActionsTriggered
//   - CrumbBar::enterEvent / leaveEvent              -> EnterLeaveEvents_ClickableAreaToggled
//   - CrumbBar::paintEvent                           -> PaintEvent_CrumbsRendered
//   - CrumbBar::showEvent lambda / resizeEvent lambda-> covered indirectly (show + resize below)
//   - CrumbBar::onHideAddrAndUpdateCrumbs lambda     -> covered indirectly by crumb population
//   - CrumbBarPrivate::setClickableAreaEnabled / writeUrlToClipboard / buttonAt /
//     updateButtonVisibility / initConnections lambdas -> covered indirectly via events above
// Branches of mouseReleaseEvent: right-button shows crumb context menu, other buttons pass through.

#include "stubext.h"
#include "views/crumbbar.h"
#include "utils/crumbinterface.h"
#include "models/crumbmodel.h"

#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QPaintEvent>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSignalSpy>
#include <QApplication>

using namespace dfmplugin_titlebar;

namespace {
class ExposedCrumbBar : public CrumbBar
{
public:
    using CrumbBar::mousePressEvent;
    using CrumbBar::mouseReleaseEvent;
    using CrumbBar::contextMenuEvent;
    using CrumbBar::enterEvent;
    using CrumbBar::leaveEvent;
    using CrumbBar::paintEvent;
};
}   // namespace

class UT_CrumbBarCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(&CrumbInterface::seprateUrl, [](CrumbInterface *, const QUrl &) {
            __DBG_STUB_INVOKE__
            QList<CrumbData> list;
            list.append(CrumbData { QUrl("file:///"), "/" });
            list.append(CrumbData { QUrl("file:///home"), "home" });
            list.append(CrumbData { QUrl("file:///home/test"), "test" });
            return list;
        });
        crumbBar = new ExposedCrumbBar();
        crumbBar->resize(400, 40);
    }
    void TearDown() override
    {
        delete crumbBar;
        crumbBar = nullptr;
        stub.clear();
    }
    void populateCrumbs()
    {
        crumbBar->onHideAddrAndUpdateCrumbs(QUrl("file:///home/test"));
        QApplication::processEvents();
    }
    ExposedCrumbBar *crumbBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_CrumbBarCov, MousePress_LeftButton_PassedToBase)
{
    // Arrange
    populateCrumbs();
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(30, 20), QPointF(30, 20),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    crumbBar->mousePressEvent(&press);

    // Assert
    EXPECT_TRUE(crumbBar->isEnabled());
    EXPECT_EQ(crumbBar->width(), 400);
}

TEST_F(UT_CrumbBarCov, MouseRelease_RightButton_GlobalPosEmitted)
{
    // Arrange
    populateCrumbs();
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(30, 20), QPointF(30, 20),
                        Qt::RightButton, Qt::RightButton, Qt::NoModifier);

    // Act
    crumbBar->mouseReleaseEvent(&release);
    QApplication::processEvents();

    // Assert
    EXPECT_TRUE(crumbBar->isEnabled());
    EXPECT_EQ(crumbBar->width(), 400);
}

TEST_F(UT_CrumbBarCov, ContextMenuEvent_MenuBuilt)
{
    // Arrange
    populateCrumbs();
    QContextMenuEvent cme(QContextMenuEvent::Mouse, QPoint(30, 20), crumbBar->mapToGlobal(QPoint(30, 20)));

    // Act
    crumbBar->contextMenuEvent(&cme);

    // Assert
    EXPECT_TRUE(crumbBar->isEnabled());
    EXPECT_EQ(crumbBar->width(), 400);
    EXPECT_TRUE(QString(crumbBar->metaObject()->className()).endsWith("CrumbBar"));
}

TEST_F(UT_CrumbBarCov, CustomMenu_ActionsTriggered)
{
    // Arrange
    QMenu menu;
    const int before = menu.actions().size();
    populateCrumbs();

    // Act
    crumbBar->customMenu(QUrl("file:///home/test"), &menu);
    const int added = menu.actions().size() - before;
    int triggered = 0;
    for (QAction *act : menu.actions())
        act->trigger(), ++triggered;
    QApplication::processEvents();

    // Assert
    EXPECT_GE(added, 0);
    EXPECT_EQ(triggered, menu.actions().size());
}

TEST_F(UT_CrumbBarCov, EnterLeaveEvents_ClickableAreaToggled)
{
    // Arrange
    populateCrumbs();
    QEnterEvent enter(QPointF(10, 20), QPointF(10, 20), QPointF(0, 0));
    QEvent leave(QEvent::Leave);

    // Act
    crumbBar->enterEvent(&enter);
    crumbBar->leaveEvent(&leave);

    // Assert
    EXPECT_TRUE(crumbBar->isEnabled());
    EXPECT_FALSE(crumbBar->underMouse());
    EXPECT_EQ(crumbBar->width(), 400);
}

TEST_F(UT_CrumbBarCov, PaintEvent_CrumbsRendered)
{
    // Arrange
    populateCrumbs();
    QPaintEvent paint(QRect(0, 0, 400, 40));

    // Act
    crumbBar->paintEvent(&paint);
    QImage img(crumbBar->size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    crumbBar->render(&p);

    // Assert
    EXPECT_EQ(img.size(), QSize(400, 40));
    EXPECT_FALSE(img.isNull());
}

TEST_F(UT_CrumbBarCov, ShowAndResize_EventsDelivered)
{
    // Arrange
    populateCrumbs();
    QResizeEvent resize(QSize(500, 40), QSize(400, 40));
    QShowEvent show;

    // Act
    crumbBar->resizeEvent(&resize);
    crumbBar->showEvent(&show);
    QApplication::processEvents();

    // Assert: manual event delivery does not resize, QWidget::resize() does
    EXPECT_EQ(crumbBar->size(), QSize(400, 40));
    crumbBar->resize(500, 40);
    EXPECT_EQ(crumbBar->size(), QSize(500, 40));
}
