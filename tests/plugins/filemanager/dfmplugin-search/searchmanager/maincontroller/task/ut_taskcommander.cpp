// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/maincontroller/task/taskcommander.h"
#include "searchmanager/maincontroller/task/taskcommander_p.h"
#include "searchmanager/searcher/anything/anythingsearcher.h"
#include "searchmanager/searcher/fsearch/fsearcher.h"
#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "searchmanager/searcher/fulltext/fulltextsearcher.h"

#include "stubext.h"

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE

class TestSearcher : public AbstractSearcher
{
public:
    explicit TestSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr)
        : AbstractSearcher(url, key, parent) {}

    bool search() { return true; }
    void stop() {}
    bool hasItem() const { return true; }
    QList<QUrl> takeAll() { return { QUrl("file:///home") }; }
};

// TaskCommanderPrivate
TEST(TaskCommanderPrivateTest, ut_working)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    TestSearcher searcher(QUrl("file:///home"), "key");

    EXPECT_NO_FATAL_FAILURE(task.d->working(&searcher));
}

TEST(TaskCommanderPrivateTest, ut_createFileNameSearcher_1)
{
    stub_ext::StubExt st;
    st.set_lamda(AnythingSearcher::isSupported, [] { return true; });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    EXPECT_NO_FATAL_FAILURE(task.d->createFileNameSearcher(QUrl("file:///home"), "key"));
}

TEST(TaskCommanderPrivateTest, ut_createFileNameSearcher_2)
{
    stub_ext::StubExt st;
    st.set_lamda(AnythingSearcher::isSupported, [] { return false; });
    st.set_lamda(FSearcher::isSupport, [] { return true; });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    EXPECT_NO_FATAL_FAILURE(task.d->createFileNameSearcher(QUrl("file:///home"), "key"));
}

TEST(TaskCommanderPrivateTest, ut_createFileNameSearcher_3)
{
    stub_ext::StubExt st;
    st.set_lamda(AnythingSearcher::isSupported, [] { return false; });
    st.set_lamda(FSearcher::isSupport, [] { return false; });
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    EXPECT_NO_FATAL_FAILURE(task.d->createFileNameSearcher(QUrl("file:///home"), "key"));
}

TEST(TaskCommanderPrivateTest, ut_onUnearthed)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    TestSearcher searcher(QUrl("file:///home"), "key");
    task.d->allSearchers << &searcher;

    EXPECT_NO_FATAL_FAILURE(task.d->onUnearthed(&searcher));
}

TEST(TaskCommanderPrivateTest, ut_onFinished_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});
    st.set_lamda(&QFutureWatcher<void>::isFinished, [] { return true; });
    st.set_lamda(&TaskCommander::deleteLater, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->deleted = true;

    EXPECT_NO_FATAL_FAILURE(task.d->onFinished());
}

TEST(TaskCommanderPrivateTest, ut_onFinished_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});
    st.set_lamda(&QFutureWatcher<void>::isFinished, [] { return true; });

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->deleted = false;

    EXPECT_NO_FATAL_FAILURE(task.d->onFinished());
}

// TaskCommander
TEST(TaskCommanderTest, ut_taskID)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    auto id = task.taskID();

    EXPECT_EQ(id, "taskId");
}

TEST(TaskCommanderTest, ut_getResults)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->resultList << QUrl("file:///home");
    auto result = task.getResults();

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(task.d->resultList.isEmpty());
}

TEST(TaskCommanderTest, ut_start_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->isWorking = true;

    EXPECT_FALSE(task.start());
}

TEST(TaskCommanderTest, ut_start_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->isWorking = false;

    EXPECT_TRUE(task.start());
    EXPECT_FALSE(task.d->isWorking);
}

TEST(TaskCommanderTest, ut_start_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});

    TestSearcher searcher(QUrl("file:///home"), "key");
    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->isWorking = false;
    task.d->allSearchers << &searcher;

    EXPECT_TRUE(task.start());
    EXPECT_TRUE(task.d->isWorking);
    task.d->futureWatcher.waitForFinished();
}

TEST(TaskCommanderTest, ut_stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});
    st.set_lamda(&QFuture<void>::cancel, [] {});

    TestSearcher searcher(QUrl("file:///home"), "key");
    TaskCommander task("taskId", QUrl("file:///home"), "key");
    task.d->allSearchers << &searcher;

    task.stop();
    EXPECT_FALSE(task.d->isWorking);
    EXPECT_TRUE(task.d->finished);
}

TEST(TaskCommanderTest, ut_deleteSelf_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});
    st.set_lamda(&QFutureWatcher<void>::isFinished, [] { return true; });

    TaskCommander *task = new TaskCommander("taskId", QUrl("file:///home"), "key");
    EXPECT_NO_FATAL_FAILURE(task->deleteSelf());
}

TEST(TaskCommanderTest, ut_deleteSelf_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&TaskCommander::createSearcher, [] {});
    st.set_lamda(&QFutureWatcher<void>::isFinished, [] { return false; });

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    EXPECT_NO_FATAL_FAILURE(task.deleteSelf());
    EXPECT_TRUE(task.d->deleted);
}

TEST(TaskCommanderTest, ut_createSearcher)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcher::isSupport, [] { return true; });

    TestSearcher searcher(QUrl(), "");
    st.set_lamda(&TaskCommanderPrivate::createFileNameSearcher, [&searcher] { return &searcher; });

    TaskCommander task("taskId", QUrl("file:///home"), "key");
    EXPECT_FALSE(task.d->allSearchers.isEmpty());
}
