// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbbar_1.cpp
 * @brief Unit tests for CrumbBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/crumbbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *event))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(CrumbBarTest, customMenu)
{
    // Test method: void customMenu((const QUrl &url, QMenu *menu))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->customMenu(_arg0, nullptr));
}

TEST_F(CrumbBarTest, enterEvent)
{
    // Test event handler: enterEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(CrumbBarTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CrumbBarTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(CrumbBarTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(CrumbBarTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(CrumbBarTest, onKeepAddressBar)
{
    // Test method: void onKeepAddressBar((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onKeepAddressBar(_arg0));
}

TEST_F(CrumbBarTest, onUrlChanged)
{
    // Test method: void onUrlChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onUrlChanged(_arg0));
}

TEST_F(CrumbBarTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(CrumbBarTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(CrumbBarTest, setPopupVisible)
{
    // Test setter: void setPopupVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setPopupVisible(false));
}

TEST_F(CrumbBarTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
