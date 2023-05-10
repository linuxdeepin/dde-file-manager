// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/maincontroller/maincontroller.h"
#include "searchmanager/maincontroller/task/taskcommander.h"
#include "searchmanager/searcher/fulltext/fulltextsearcher.h"

#include "stubext.h"

#include <dfm-base/base/application/settings.h>
#include <gtest/gtest.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

TEST(MainControllerTest, ut_stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::stop, [] {});
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    MainController mc;
    mc.taskManager.insert("test", new TaskCommander("test", QUrl("file:///home"), "key"));

    EXPECT_NO_FATAL_FAILURE(mc.stop("test"));
}

TEST(MainControllerTest, ut_doSearchTask_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::start, [] { return true; });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    MainController mc;
    bool ret = mc.doSearchTask("taskId", QUrl("file:///home"), "key");

    EXPECT_TRUE(ret);
    EXPECT_TRUE(!mc.taskManager.isEmpty());
}

TEST(MainControllerTest, ut_doSearchTask_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::start, [] { return false; });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    MainController mc;
    bool ret = mc.doSearchTask("taskId", QUrl("file:///home"), "key");

    EXPECT_FALSE(ret);
}

TEST(MainControllerTest, ut_getResults)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::getResults, [] { return QList<QUrl>(); });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    MainController mc;
    mc.taskManager.insert("test", new TaskCommander("test", QUrl("file:///home"), "key"));
    auto list = mc.getResults("test");

    EXPECT_TRUE(list.isEmpty());
}

TEST(MainControllerTest, ut_onFinished)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    MainController mc;
    mc.taskManager.insert("test", new TaskCommander("test", QUrl("file:///home"), "key"));

    EXPECT_NO_FATAL_FAILURE(mc.onFinished("test"));
}

// TODO(liuzhangjian):
//TEST(MainControllerTest, ut_onFileChanged)
//{
//    stub_ext::StubExt st;
//    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
//    auto value = static_cast<Value>(&Settings::value);
//    st.set_lamda(value, [] { return true; });
//    st.set_lamda(&QFuture<void>::isFinished, [] { return false; });
//    st.set_lamda(&FullTextSearcher::createIndex, [] { return true; });

//    MainController mc;
//    EXPECT_NO_FATAL_FAILURE(mc.onFileChanged(""));
//    mc.indexFuture.waitForFinished();
//}
