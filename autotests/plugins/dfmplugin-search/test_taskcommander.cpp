// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QThread>
#include <QMetaObject>
#include <QMutex>
#include <QReadWriteLock>

#include "searchmanager/maincontroller/task/taskcommander.h"
#include "searchmanager/maincontroller/task/taskcommander_p.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-search/dsearch_global.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

// Mock AbstractSearcher for testing
class MockSearcher : public AbstractSearcher
{
public:
    explicit MockSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr)
        : AbstractSearcher(url, key, parent), m_hasItem(false), m_searchCalled(false), m_stopCalled(false)
    {
    }

    bool search() override
    {
        m_searchCalled = true;
        return true;
    }

    void stop() override
    {
        m_stopCalled = true;
    }

    bool hasItem() const override
    {
        return m_hasItem;
    }

    DFMSearchResultMap takeAll() override
    {
        DFMSearchResultMap results = m_results;
        m_results.clear();
        m_hasItem = false;
        return results;
    }

    void addMockResult(const QUrl &url, double score = 1.0)
    {
        DFMSearchResult result(url);
        result.setMatchScore(score);
        m_results.insert(url, result);
        m_hasItem = true;
    }

    void emitUnearthed()
    {
        emit unearthed(this);
    }

    void emitFinished()
    {
        emit finished();
    }

    bool m_hasItem;
    bool m_searchCalled;
    bool m_stopCalled;
    DFMSearchResultMap m_results;
};

class TestTaskCommanderPrivate : public testing::Test
{
public:
    void SetUp() override
    {
        taskId = "test-task-123";
        searchUrl = QUrl::fromLocalFile("/home/test");
        keyword = "test";
    }

    void TearDown() override
    {
        if (commander) {
            delete commander;
            commander = nullptr;
        }
        stub.clear();
    }

    void setupBasicStubs()
    {
        // Stub QThread operations
        stub.set_lamda(&QThread::start, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QThread::quit, [](QThread *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(qOverload<QDeadlineTimer>(&QThread::wait), [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&QThread::isRunning, [](const QThread *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub QMetaObject::invokeMethod
        stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                            Qt::ConnectionType, qsizetype,
                                            const void *const *, const char *const *,
                                            const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                       [](QObject *, QtPrivate::QSlotObjectBase *, Qt::ConnectionType,
                          qsizetype, const void *const *, const char *const *,
                          const QtPrivate::QMetaTypeInterface *const *) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        // Stub DFMSearcher and IteratorSearcher constructors
        stub.set_lamda(&DFMSearcher::supportUrl, [](const QUrl &url) -> bool {
            __DBG_STUB_INVOKE__
            return url.scheme() == "file";
        });

        stub.set_lamda(&DFMSearcher::realSearchPath, [](const QUrl &url) -> QString {
            __DBG_STUB_INVOKE__
            return url.toLocalFile();
        });

        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList() << "/home/test/Documents";
        });

        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) -> QVariant {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });
    }

protected:
    stub_ext::StubExt stub;
    TaskCommander *commander = nullptr;
    QString taskId;
    QUrl searchUrl;
    QString keyword;
};

class TestTaskCommander : public testing::Test
{
public:
    void SetUp() override
    {
        taskId = "test-task-456";
        searchUrl = QUrl::fromLocalFile("/home/test");
        keyword = "search";
    }

    void TearDown() override
    {
        if (commander) {
            delete commander;
            commander = nullptr;
        }
        stub.clear();
    }

    void setupBasicStubs()
    {
        // Stub QThread operations
        stub.set_lamda(&QThread::start, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QThread::quit, [](QThread *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(qOverload<QDeadlineTimer>(&QThread::wait), [] {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&QThread::isRunning, [](const QThread *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub QMetaObject::invokeMethod
        stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                            Qt::ConnectionType, qsizetype,
                                            const void *const *, const char *const *,
                                            const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                       [](QObject *, QtPrivate::QSlotObjectBase *, Qt::ConnectionType,
                          qsizetype, const void *const *, const char *const *,
                          const QtPrivate::QMetaTypeInterface *const *) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(&DFMSearcher::supportUrl, [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList();
        });

        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) -> QVariant {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });
    }

protected:
    stub_ext::StubExt stub;
    TaskCommander *commander = nullptr;
    QString taskId;
    QUrl searchUrl;
    QString keyword;
};

// ========== TaskCommanderPrivate Tests ==========
TEST_F(TestTaskCommanderPrivate, Constructor_InitializesWorkerThread)
{
    setupBasicStubs();

    bool threadStarted = false;
    stub.set_lamda(&QThread::start, [&threadStarted] {
        __DBG_STUB_INVOKE__
        threadStarted = true;
    });

    commander = new TaskCommander(taskId, searchUrl, keyword);

    EXPECT_TRUE(threadStarted);
}

TEST_F(TestTaskCommanderPrivate, OnResultsUpdated_EmitsMatchedSignal)
{
    setupBasicStubs();

    commander = new TaskCommander(taskId, searchUrl, keyword);

    QSignalSpy spy(commander, &TaskCommander::matched);

    // Trigger resultsUpdated
    emit commander->d->searchWorker->resultsUpdated(taskId);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), taskId);
}

// ========== TaskCommander Tests ==========
TEST_F(TestTaskCommander, Constructor_WithValidParameters_CreatesInstance)
{
    setupBasicStubs();

    commander = new TaskCommander(taskId, searchUrl, keyword);

    EXPECT_NE(commander, nullptr);
    EXPECT_NE(commander->d, nullptr);
    EXPECT_NE(commander->d->searchWorker, nullptr);
}

TEST_F(TestTaskCommander, TaskID_ReturnsCorrectTaskId)
{
    setupBasicStubs();

    commander = new TaskCommander(taskId, searchUrl, keyword);

    EXPECT_EQ(commander->taskID(), taskId);
}

TEST_F(TestTaskCommander, GetResults_WithNoResults_ReturnsEmptyMap)
{
    setupBasicStubs();

    commander = new TaskCommander(taskId, searchUrl, keyword);

    DFMSearchResultMap results = commander->getResults();

    EXPECT_TRUE(results.isEmpty());
}

TEST_F(TestTaskCommander, GetResults_WithResults_ReturnsResults)
{
    setupBasicStubs();

    bool invokeMethodCalled = false;
    stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                        Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&invokeMethodCalled](QObject *, QtPrivate::QSlotObjectBase *,
                                         Qt::ConnectionType, qsizetype,
                                         const void *const *, const char *const *,
                                         const QtPrivate::QMetaTypeInterface *const *) -> bool {
                       __DBG_STUB_INVOKE__
                       invokeMethodCalled = true;
                       return true;
                   });

    commander = new TaskCommander(taskId, searchUrl, keyword);

    commander->getResults();

    EXPECT_TRUE(invokeMethodCalled);
}

TEST_F(TestTaskCommander, GetResultsUrls_WithNoResults_ReturnsEmptyList)
{
    setupBasicStubs();

    commander = new TaskCommander(taskId, searchUrl, keyword);

    QList<QUrl> urls = commander->getResultsUrls();

    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(TestTaskCommander, GetResultsUrls_InvokesWorkerMethod)
{
    setupBasicStubs();

    bool invokeMethodCalled = false;
    stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                        Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&invokeMethodCalled](QObject *, QtPrivate::QSlotObjectBase *,
                                         Qt::ConnectionType, qsizetype,
                                         const void *const *, const char *const *,
                                         const QtPrivate::QMetaTypeInterface *const *) -> bool {
                       __DBG_STUB_INVOKE__
                       invokeMethodCalled = true;
                       return true;
                   });

    commander = new TaskCommander(taskId, searchUrl, keyword);

    commander->getResultsUrls();

    EXPECT_TRUE(invokeMethodCalled);
}

TEST_F(TestTaskCommander, Start_InvokesWorkerStartSearch)
{
    setupBasicStubs();

    bool startSearchCalled = false;
    stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                        Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&startSearchCalled](QObject *, QtPrivate::QSlotObjectBase *,
                                        Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *) -> bool {
                       __DBG_STUB_INVOKE__
                       startSearchCalled = true;
                       return true;
                   });

    commander = new TaskCommander(taskId, searchUrl, keyword);

    bool result = commander->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(startSearchCalled);
}

TEST_F(TestTaskCommander, Stop_InvokesWorkerStopSearch)
{
    setupBasicStubs();

    bool stopSearchCalled = false;
    stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                        Qt::ConnectionType, qsizetype,
                                        const void *const *, const char *const *,
                                        const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                   [&stopSearchCalled](QObject *, QtPrivate::QSlotObjectBase *,
                                       Qt::ConnectionType, qsizetype,
                                       const void *const *, const char *const *,
                                       const QtPrivate::QMetaTypeInterface *const *) -> bool {
                       __DBG_STUB_INVOKE__
                       stopSearchCalled = true;
                       return true;
                   });

    commander = new TaskCommander(taskId, searchUrl, keyword);

    commander->stop();

    EXPECT_TRUE(stopSearchCalled);
}

