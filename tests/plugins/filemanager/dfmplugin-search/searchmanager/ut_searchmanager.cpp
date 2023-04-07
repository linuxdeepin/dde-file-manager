// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/searchmanager.h"
#include "searchmanager/maincontroller/maincontroller.h"
#include "stubext.h"

#include <dfm-framework/dpf.h>

#include <QUrl>

#include <gtest/gtest.h>

#define SearchManagerIns SearchManager::instance()

DPF_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

TEST(SearchManagerTest, ut_search)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::doSearchTask, [] { return true; });

    bool ret = SearchManagerIns->search(1, "taskId", QUrl("file:///home"), "test");
    EXPECT_TRUE(ret);
}

TEST(SearchManagerTest, ut_matchedResults)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::getResults, [] { return QList<QUrl>(); });

    auto list = SearchManagerIns->matchedResults("test");
    EXPECT_TRUE(list.isEmpty());
}

TEST(SearchManagerTest, ut_stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::stop, [] { return; });

    SearchManagerIns->taskIdMap[1] = "test";

    EXPECT_NO_FATAL_FAILURE(SearchManagerIns->stop(1));
}

TEST(SearchManagerTest, ut_onIndexFullTextConfigChanged)
{
    stub_ext::StubExt st;
    typedef bool (EventDispatcherManager::*Publish)(const QString &, const QString &, QString, QVariantMap&);

    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    st.set_lamda(publish, [] { return true; });

    EXPECT_NO_FATAL_FAILURE(SearchManagerIns->onIndexFullTextConfigChanged(true));
}
