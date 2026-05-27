// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/bookmarkeventcaller.h"

#include <dfm-framework/dpf.h>

#include <QUrl>

using namespace dfmplugin_bookmark;
DPF_USE_NAMESPACE

class UT_BookMarkEventCaller : public testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_BookMarkEventCaller, SendBookMarkOpenInNewWindow)
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

TEST_F(UT_BookMarkEventCaller, SendBookMarkOpenInNewTab)
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

TEST_F(UT_BookMarkEventCaller, SendShowBookMarkPropertyDialog)
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

TEST_F(UT_BookMarkEventCaller, SendOpenBookMarkInWindow)
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

TEST_F(UT_BookMarkEventCaller, SendCheckTabAddable)
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

