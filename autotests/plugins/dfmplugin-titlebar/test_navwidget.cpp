// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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

// ===== PMS task 398283: tests for navigateWithCheck (async network check) =====
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include "events/titlebareventcaller.h"

DFMBASE_USE_NAMESPACE

TEST_F(NavWidgetTest, NavigateWithCheck_InvalidUrl_NoCrash)
{
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(false);
                   });
    stub.set_lamda(&TitleBarEventCaller::sendCd, [](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    QUrl url;
    EXPECT_NO_FATAL_FAILURE({ widget->navigateWithCheck(url); });

    for (int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
}

TEST_F(NavWidgetTest, NavigateWithCheck_BusyUrl_NoSendCd)
{
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(true);
                   });
    stub.set_lamda(&DialogManager::showUnableToVistDir, [](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    bool sendCdCalled = false;
    stub.set_lamda(&TitleBarEventCaller::sendCd, [&sendCdCalled](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        sendCdCalled = true;
    });

    widget->navigateWithCheck(QUrl("file:///run/user/1000/gvfs/ftp:host=1.2.3.4"));

    for (int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    EXPECT_FALSE(sendCdCalled);
}

TEST_F(NavWidgetTest, NavigateWithCheck_NotBusyUrl_CallsSendCd)
{
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(false);
                   });

    bool sendCdCalled = false;
    stub.set_lamda(&TitleBarEventCaller::sendCd, [&sendCdCalled](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        sendCdCalled = true;
    });

    widget->navigateWithCheck(QUrl("file:///home/test"));

    for (int i = 0; i < 10 && !sendCdCalled; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    EXPECT_TRUE(sendCdCalled);
}
