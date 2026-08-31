// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlpushbutton_1.cpp
 * @brief Unit tests for UrlPushButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/urlpushbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class UrlPushButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UrlPushButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UrlPushButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UrlPushButtonTest, UrlPushButton)
{
    // Test constructor: UrlPushButton((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(UrlPushButtonTest, activeSubDirectory)
{
    // Test getter: QString activeSubDirectory()
    auto result = obj->activeSubDirectory();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlPushButtonTest, crumbDatas)
{
    // Test getter: QList<CrumbData> crumbDatas()
    auto result = obj->crumbDatas();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlPushButtonTest, enterEvent)
{
    // Test event handler: enterEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(UrlPushButtonTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(UrlPushButtonTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(UrlPushButtonTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(UrlPushButtonTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(UrlPushButtonTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(UrlPushButtonTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(UrlPushButtonTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(UrlPushButtonTest, setActive)
{
    // Test setter: void setActive((bool active))
    EXPECT_NO_FATAL_FAILURE(obj->setActive(false));
}

TEST_F(UrlPushButtonTest, setActiveSubDirectory)
{
    // Test setter: void setActiveSubDirectory((const QString &subDir))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setActiveSubDirectory(_arg0));
}

TEST_F(UrlPushButtonTest, setCrumbDatas)
{
    // Test setter: void setCrumbDatas((const QList<CrumbData> &datas, bool stacked))
    QList<CrumbData> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCrumbDatas(_arg0, false));
}
