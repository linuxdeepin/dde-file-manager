// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "events/smbbrowsereventcaller.h"

#include <dfm-framework/dpf.h>

#include <QUrl>

DPSMBBROWSER_USE_NAMESPACE

class UT_SmbBrowserEventCaller : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SmbBrowserEventCaller, SendOpenWindow)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType, QUrl);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendOpenWindow({}));
    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendOpenWindow(QUrl::fromLocalFile("/")));
}

TEST_F(UT_SmbBrowserEventCaller, SendOpenTab)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType, quint64, const QUrl &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendOpenTab(0, {}));
    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendOpenTab(0, QUrl::fromLocalFile("/")));
}

TEST_F(UT_SmbBrowserEventCaller, SendCheckTabAddable)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendCheckTabAddable(0));
    EXPECT_TRUE(SmbBrowserEventCaller::sendCheckTabAddable(0));
}

TEST_F(UT_SmbBrowserEventCaller, SendChangeCurrentUrl)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType, quint64, const QUrl &);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendChangeCurrentUrl(0, {}));
    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendChangeCurrentUrl(0, QUrl::fromLocalFile("/")));
}

TEST_F(UT_SmbBrowserEventCaller, SendShowPropertyDialog)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendShowPropertyDialog({}));
    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventCaller::sendShowPropertyDialog(QUrl::fromLocalFile("/")));
}
