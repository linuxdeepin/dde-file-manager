// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "iterator/searchdiriterator.h"
#include "iterator/searchdiriterator_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "events/searcheventcaller.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/utils/fileutils.h>

#include <QUrl>

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE
using namespace dfmbase;

TEST(UT_SearchPluginBugTest, bug_173015_CanNotNewVistualPathWatcher)
{
    const auto &searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    SearchDirIterator it(searchUrl);

    bool isStartWatcher { false };
    stub_ext::StubExt st;
    st.set_lamda(&LocalFileWatcherPrivate::initFileWatcher, [] {});
    st.set_lamda(&LocalFileWatcherPrivate::initConnect, [] {});
    st.set_lamda(&CustomManager::isDisableSearch, [] { return false; });
    st.set_lamda(&CustomManager::redirectedPath, [] { return "/home"; });
    st.set_lamda(&SearchEventCaller::sendStartSpinner, [] {});
    st.set_lamda(&SearchManager::search, [] { return true; });
    st.set_lamda(&SearchHelper::searchTargetUrl, [] { __DBG_STUB_INVOKE__ return QUrl("dfmvault:///"); });
    st.set_lamda(VADDR(AbstractFileWatcher,startWatcher), [&isStartWatcher](){ isStartWatcher = true; return true; });

    it.d->doSearch();
    EXPECT_FALSE(isStartWatcher);
    EXPECT_FALSE(dfmbase::FileUtils::isLocalFile(QUrl("dfmvault:///")));
}
