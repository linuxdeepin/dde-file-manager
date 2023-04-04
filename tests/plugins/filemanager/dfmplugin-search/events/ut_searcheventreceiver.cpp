// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "events/searcheventreceiver.h"
#include "events/searcheventcaller.h"
#include "searchmanager/searchmanager.h"
#include "utils/searchhelper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include "stubext.h"

#include <QUrl>

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(SearchEventReceiverTest, ut_handleSearch)
{
    stub_ext::StubExt st;
    FileManagerWindow fmw(QUrl::fromLocalFile("/home"));
    st.set_lamda(&FileManagerWindowsManager::findWindowById, [&fmw] {
        return &fmw;
    });
    st.set_lamda(&FileManagerWindow::currentUrl, [] {
        return QUrl::fromLocalFile("/home");
    });
    st.set_lamda(&SearchEventCaller::sendChangeCurrentUrl, [] { return; });

    EXPECT_NO_FATAL_FAILURE(SearchEventReceiverIns->handleSearch(123, "test"));
}

TEST(SearchEventReceiverTest, ut_handleStopSearch)
{
    stub_ext::StubExt st;
    typedef void (SearchManager::*Stop)(quint64);
    auto stop = static_cast<Stop>(&SearchManager::stop);
    st.set_lamda(stop, [] { return; });

    EXPECT_NO_FATAL_FAILURE(SearchEventReceiverIns->handleStopSearch(123));
}

TEST(SearchEventReceiverTest, ut_handleShowAdvanceSearchBar)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchEventCaller::sendShowAdvanceSearchBar, [] { return; });

    EXPECT_NO_FATAL_FAILURE(SearchEventReceiverIns->handleShowAdvanceSearchBar(123, true));
}

TEST(SearchEventReceiverTest, ut_handleUrlChanged)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchEventReceiver::handleStopSearch, [] { return; });

    QUrl url = SearchHelper::fromSearchFile("/home");
    EXPECT_NO_FATAL_FAILURE(SearchEventReceiverIns->handleUrlChanged(123, url));
}
