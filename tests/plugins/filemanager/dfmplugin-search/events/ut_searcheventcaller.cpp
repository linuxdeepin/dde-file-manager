// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "events/searcheventcaller.h"

#include "stubext.h"

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

TEST(SearchEventCallerTest, ut_sendChangeCurrentUrl)
{
    stub_ext::StubExt st;
    typedef bool (EventDispatcherManager::*Publish)(EventType, quint64, const QUrl &);
    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    st.set_lamda(publish, [] { return true; });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendChangeCurrentUrl(123, QUrl()));
}

TEST(SearchEventCallerTest, ut_sendShowAdvanceSearchBar)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, QString &&, bool &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendShowAdvanceSearchBar(123, true));
}

TEST(SearchEventCallerTest, ut_sendShowAdvanceSearchButton)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, bool &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendShowAdvanceSearchButton(123, true));
}

TEST(SearchEventCallerTest, ut_sendStartSpinner)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendStartSpinner(123));
}

TEST(SearchEventCallerTest, ut_sendStopSpinner)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(SearchEventCaller::sendStopSpinner(123));
}
