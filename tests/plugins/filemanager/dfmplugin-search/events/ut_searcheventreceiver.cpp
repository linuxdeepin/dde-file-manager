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
#include "events/searcheventreceiver.h"
#include "events/searcheventcaller.h"
#include "searchmanager/searchmanager.h"
#include "utils/searchhelper.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

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
