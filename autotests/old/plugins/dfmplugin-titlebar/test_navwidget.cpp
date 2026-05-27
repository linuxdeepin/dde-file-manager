// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/navwidget.h"
#include "views/private/navwidget_p.h"
#include "utils/historystack.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QTest>

using namespace dfmplugin_titlebar;

class NavWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        widget = new NavWidget();
        stub.clear();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    NavWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(NavWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(NavWidgetTest, PushUrlToHistoryStack_ValidUrl_UrlPushed)
{
    QUrl url("file:///home/test");

    // Stub HistoryStack operations
    stub.set_lamda(&HistoryStack::append, [](HistoryStack *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    widget->pushUrlToHistoryStack(url);
    // Verify it doesn't crash
}

TEST_F(NavWidgetTest, RemoveUrlFromHistoryStack_ValidUrl_UrlRemoved)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&HistoryStack::removeUrl, [](HistoryStack *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    widget->removeUrlFromHistoryStack(url);
}

TEST_F(NavWidgetTest, AddHistroyStack_Called_StackAdded)
{
    widget->addHistroyStack();
    // Verify it doesn't crash
}

TEST_F(NavWidgetTest, SwitchHistoryStack_ValidIndex_StackSwitched)
{
    widget->addHistroyStack();
    widget->addHistroyStack();

    stub.set_lamda(&HistoryStack::backIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&HistoryStack::forwardIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget->switchHistoryStack(1);
}

TEST_F(NavWidgetTest, Back_WithHistory_NavigatesBack)
{
    stub.set_lamda(&HistoryStack::back, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/previous");
    });
    stub.set_lamda(&HistoryStack::backIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&HistoryStack::forwardIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget->back();
}

TEST_F(NavWidgetTest, Forward_WithHistory_NavigatesForward)
{
    stub.set_lamda(&HistoryStack::forward, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/next");
    });
    stub.set_lamda(&HistoryStack::backIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&HistoryStack::forwardIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget->forward();
}

TEST_F(NavWidgetTest, OnUrlChanged_ValidUrl_UrlUpdated)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&HistoryStack::append, [](HistoryStack *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&HistoryStack::backIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&HistoryStack::forwardIsExist, [](HistoryStack *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget->onUrlChanged(url);
}

TEST_F(NavWidgetTest, MoveNavStacks_ValidIndices_StacksMoved)
{
    widget->addHistroyStack();
    widget->addHistroyStack();
    widget->addHistroyStack();

    widget->moveNavStacks(0, 2);
}

TEST_F(NavWidgetTest, RemoveNavStackAt_ValidIndex_StackRemoved)
{
    widget->addHistroyStack();
    widget->addHistroyStack();

    widget->removeNavStackAt(0);
}
