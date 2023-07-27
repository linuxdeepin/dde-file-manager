// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/bookmarkeventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>
#include <QRect>
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_bookmark;

TEST(UT_BookMarkEventCaller, sendOpenWindow)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    BookMarkEventCaller::sendBookMarkOpenInNewWindow(QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_BookMarkEventCaller, sendBookMarkOpenInNewTab)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    BookMarkEventCaller::sendBookMarkOpenInNewTab(1, QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_BookMarkEventCaller, sendShowBookMarkPropertyDialog)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });
    BookMarkEventCaller::sendShowBookMarkPropertyDialog(QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_BookMarkEventCaller, sendOpenBookMarkInWindow)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [&isCall] {
        isCall = true;
        return true;
    });

    BookMarkEventCaller::sendOpenBookMarkInWindow(1, QUrl());

    EXPECT_TRUE(isCall);
}

TEST(UT_BookMarkEventCaller, sendCheckTabAddable)
{
    bool isCall { false };

    stub_ext::StubExt stub;
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });
    BookMarkEventCaller::sendCheckTabAddable(0);

    EXPECT_TRUE(isCall);
}
