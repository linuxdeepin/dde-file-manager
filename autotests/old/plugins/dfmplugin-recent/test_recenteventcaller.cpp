// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/recenteventcaller.h"
#include <dfm-base/base/application/application.h>
#include <dfm-framework/event/event.h>
#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DPF_USE_NAMESPACE
        using namespace dfmplugin_recent;

class RecentEventCallerTest : public testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(RecentEventCallerTest, sendOpenWindow)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const QUrl &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendOpenWindow(QUrl()));
}

TEST_F(RecentEventCallerTest, sendOpenTab)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, const QUrl &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendOpenTab(123, QUrl()));
}

TEST_F(RecentEventCallerTest, sendOpenFiles)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const quint64, const QList<QUrl> &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendOpenFiles(123, QList<QUrl>()));
}

TEST_F(RecentEventCallerTest, sendWriteToClipboard)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const quint64, const QList<QUrl> &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendWriteToClipboard(123, ClipBoard::ClipboardAction::kCopyAction, QList<QUrl>()));
}

TEST_F(RecentEventCallerTest, sendCopyFiles)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const quint64, const QList<QUrl> &, const QUrl &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendCopyFiles(123, QList<QUrl>(), QUrl(), {}));
}

TEST_F(RecentEventCallerTest, sendCutFiles)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const quint64, const QList<QUrl> &, const QUrl &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendCutFiles(123, QList<QUrl>(), QUrl(), {}));
}

TEST_F(RecentEventCallerTest, sendCheckTabAddable)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, const quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(RecentEventCaller::sendCheckTabAddable(123));
}
