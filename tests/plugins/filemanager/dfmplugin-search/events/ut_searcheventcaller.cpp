/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
