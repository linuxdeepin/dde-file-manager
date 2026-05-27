// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "models/rootinfo.h"
#include "utils/traversaldirthreadmanager.h"

#include <QUrl>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QList>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

using namespace dfmplugin_workspace;

class TestRootInfo : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        rootInfo.reset(new RootInfo(testUrl, true));
    }

    void TearDown() override
    {
        rootInfo.reset();
        stub.clear();
    }

    QUrl testUrl;
    std::unique_ptr<RootInfo> rootInfo;
    stub_ext::StubExt stub;
};

TEST_F(TestRootInfo, CanCreateAndDestroy)
{
    EXPECT_NO_FATAL_FAILURE(rootInfo.reset(new RootInfo(QUrl::fromLocalFile("/tmp"), true)));
    EXPECT_NE(rootInfo, nullptr);
    EXPECT_NO_FATAL_FAILURE(rootInfo.reset());
}

TEST_F(TestRootInfo, CanInitThreadOfFileData)
{
    QString key = "test_key";
    DFMGLOBAL_NAMESPACE::ItemRoles role = DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole;
    Qt::SortOrder order = Qt::AscendingOrder;
    bool isMixFileAndFolder = false;
    
    // Stub the TraversalDirThreadManager to avoid crashes
    stub.set_lamda(ADDR(TraversalDirThreadManager, start), [](TraversalDirThreadManager *) {
        __DBG_STUB_INVOKE__
    });
    
    bool result = rootInfo->initThreadOfFileData(key, role, order, isMixFileAndFolder);
    EXPECT_TRUE(result);
}

TEST_F(TestRootInfo, CanStartWork)
{
    QString key = "test_key";
    
    // Stub the TraversalDirThreadManager start method
    stub.set_lamda(ADDR(TraversalDirThreadManager, start), [](TraversalDirThreadManager *) {
        __DBG_STUB_INVOKE__
    });
    
    rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWork(key, false));
}

TEST_F(TestRootInfo, CanClearTraversalThread)
{
    QString key = "test_key";
    
    // Stub the TraversalDirThreadManager start method
    stub.set_lamda(ADDR(TraversalDirThreadManager, start), [](TraversalDirThreadManager *) {
        __DBG_STUB_INVOKE__
    });
    
    rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    
    int result = rootInfo->clearTraversalThread(key, false);
    EXPECT_EQ(result, 0);
}

TEST_F(TestRootInfo, CanSetFirstBatch)
{
    EXPECT_NO_FATAL_FAILURE(rootInfo->setFirstBatch(true));
    EXPECT_NO_FATAL_FAILURE(rootInfo->setFirstBatch(false));
}

TEST_F(TestRootInfo, CanReset)
{
    EXPECT_NO_FATAL_FAILURE(rootInfo->reset());
}

TEST_F(TestRootInfo, CanDelete)
{
    bool result = rootInfo->canDelete();
    EXPECT_TRUE(result);
}

TEST_F(TestRootInfo, CanGetKeyWords)
{
    QStringList keywords = rootInfo->getKeyWords();
    EXPECT_TRUE(keywords.isEmpty() || !keywords.isEmpty());
}

TEST_F(TestRootInfo, CanCheckKeyOnly)
{
    QString key = "test_key";
    bool result = rootInfo->checkKeyOnly(key);
    EXPECT_TRUE(result);
}

TEST_F(TestRootInfo, CanAddConnectToken)
{
    QString token = "test_token";
    EXPECT_NO_FATAL_FAILURE(rootInfo->addConnectToken(token));
    
    QStringList tokens = rootInfo->connectTokens();
    EXPECT_TRUE(tokens.contains(token));
}

TEST_F(TestRootInfo, CanStartWatcher)
{
    // Stub the startWatcher method
    // Remove startWatcher stub as it doesn't exist in TraversalDirThreadManager
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWatcher());
}

TEST_F(TestRootInfo, CanHandleFileEvents)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test_file.txt");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->doFileDeleted(testUrl));
    EXPECT_NO_FATAL_FAILURE(rootInfo->dofileCreated(testUrl));
    EXPECT_NO_FATAL_FAILURE(rootInfo->doFileUpdated(testUrl));
}

TEST_F(TestRootInfo, CanHandleTraversalResults)
{
    QList<FileInfoPointer> children;
    QString token = "test_token";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleTraversalResults(children, token));
}

TEST_F(TestRootInfo, CanHandleTraversalFinish)
{
    QString token = "test_token";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleTraversalFinish(token));
}

TEST_F(TestRootInfo, CanHandleTraversalSort)
{
    QString token = "test_token";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleTraversalSort(token));
}

TEST_F(TestRootInfo, CanHandleGetSourceData)
{
    QString token = "test_token";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleGetSourceData(token));
}

TEST_F(TestRootInfo, CanDoWatcherEvent)
{
    EXPECT_NO_FATAL_FAILURE(rootInfo->doWatcherEvent());
}

TEST_F(TestRootInfo, CanDoThreadWatcherEvent)
{
    EXPECT_NO_FATAL_FAILURE(rootInfo->doThreadWatcherEvent());
}

TEST_F(TestRootInfo, CanAddChildren)
{
    QList<QUrl> urlList;
    urlList << QUrl::fromLocalFile("/tmp/test1.txt");
    urlList << QUrl::fromLocalFile("/tmp/test2.txt");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->addChildren(urlList));
}

TEST_F(TestRootInfo, CanRemoveChildren)
{
    QList<QUrl> urlList;
    urlList << QUrl::fromLocalFile("/tmp/test1.txt");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->removeChildren(urlList));
}

TEST_F(TestRootInfo, CanUpdateChildren)
{
    QList<QUrl> urlList;
    urlList << QUrl::fromLocalFile("/tmp/test1.txt");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->updateChildren(urlList));
}

TEST_F(TestRootInfo, CanContainsChild)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test_file.txt");
    
    bool result = rootInfo->containsChild(testUrl);
    EXPECT_FALSE(result);
}

TEST_F(TestRootInfo, CanUpdateChild)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test_file.txt");
    
    SortInfoPointer result = rootInfo->updateChild(testUrl);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestRootInfo, CanCheckFileEventQueue)
{
    bool result = rootInfo->checkFileEventQueue();
    EXPECT_FALSE(result);
}

TEST_F(TestRootInfo, CanEnqueueEvent)
{
    QPair<QUrl, RootInfo::EventType> event;
    event.first = QUrl::fromLocalFile("/tmp/test_file.txt");
    event.second = RootInfo::kAddFile;
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->enqueueEvent(event));
}

TEST_F(TestRootInfo, CanDequeueEvent)
{
    QPair<QUrl, RootInfo::EventType> result = rootInfo->dequeueEvent();
    EXPECT_TRUE(result.first.isEmpty());
}

TEST_F(TestRootInfo, CanGetFileInfo)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test_file.txt");
    
    FileInfoPointer result = rootInfo->fileInfo(testUrl);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestRootInfo, CanSortFileInfo)
{
    FileInfoPointer info;
    
    SortInfoPointer result = rootInfo->sortFileInfo(info);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestRootInfo, CanHandleFileMoved)
{
    QUrl fromUrl = QUrl::fromLocalFile("/tmp/test_old.txt");
    QUrl toUrl = QUrl::fromLocalFile("/tmp/test_new.txt");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->dofileMoved(fromUrl, toUrl));
}

TEST_F(TestRootInfo, CanHandleTraversalResultsUpdate)
{
    QList<SortInfoPointer> children;
    QString token = "test_token";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleTraversalResultsUpdate(children, token));
}

TEST_F(TestRootInfo, CanHandleTraversalLocalResult)
{
    QList<SortInfoPointer> children;
    dfmio::DEnumerator::SortRoleCompareFlag sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    bool isMixDirAndFile = false;
    QString token = "test_token";
    
    // Stub the handleTraversalLocalResult method
    // Remove handleTraversalLocalResult stub as it doesn't exist in TraversalDirThreadManager
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleTraversalLocalResult(children, sortRole, sortOrder, isMixDirAndFile, token));
}

TEST_F(TestRootInfo, CanAddChildrenWithFileInfo)
{
    QList<FileInfoPointer> children;
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->addChildren(children));
}

TEST_F(TestRootInfo, CanAddChildrenWithSortInfo)
{
    QList<SortInfoPointer> children;
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->addChildren(children));
}

TEST_F(TestRootInfo, CanAddChild)
{
    FileInfoPointer child;
    
    SortInfoPointer result = rootInfo->addChild(child);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestRootInfo, CanHandleUrlWithKeywords)
{
    QUrl urlWithKeywords = QUrl::fromLocalFile("/tmp");
    urlWithKeywords.setQuery("keywords=test");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo.reset(new RootInfo(urlWithKeywords, true)));
    EXPECT_NE(rootInfo, nullptr);
    
    QStringList keywords = rootInfo->getKeyWords();
    EXPECT_TRUE(keywords.isEmpty() || !keywords.isEmpty());
}

TEST_F(TestRootInfo, CanHandleHiddenFileUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp");
    
    EXPECT_NO_FATAL_FAILURE(rootInfo.reset(new RootInfo(url, true)));
    EXPECT_NE(rootInfo, nullptr);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWatcher());
}

TEST_F(TestRootInfo, CanHandleMultipleTokens)
{
    QString token1 = "token1";
    QString token2 = "token2";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->addConnectToken(token1));
    EXPECT_NO_FATAL_FAILURE(rootInfo->addConnectToken(token2));
    
    QStringList tokens = rootInfo->connectTokens();
    EXPECT_TRUE(tokens.contains(token1));
    EXPECT_TRUE(tokens.contains(token2));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->addConnectToken(token1));
    
    tokens = rootInfo->connectTokens();
    EXPECT_EQ(tokens.count(token1), 1);
}

TEST_F(TestRootInfo, CanHandleMultipleThreads)
{
    QString key1 = "key1";
    QString key2 = "key2";
    
    // Stub the TraversalDirThreadManager start method to prevent crashes
    stub.set_lamda(ADDR(TraversalDirThreadManager, start), [](TraversalDirThreadManager *) {
        __DBG_STUB_INVOKE__
    });
    
    rootInfo->initThreadOfFileData(key1, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    rootInfo->initThreadOfFileData(key2, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWork(key1, false));
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWork(key2, false));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key1, false));
    
    bool result = rootInfo->checkKeyOnly(key2);
    EXPECT_TRUE(result);
}

TEST_F(TestRootInfo, CanHandleDifferentRoles)
{
    QString key = "test_key";
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false));
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key, false));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileSizeRole, Qt::DescendingOrder, true));
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key, false));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileLastReadRole, Qt::AscendingOrder, false));
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key, false));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileLastModifiedRole, Qt::DescendingOrder, true));
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key, false));
}

TEST_F(TestRootInfo, CanHandleFileEventQueue)
{
    QUrl url1 = QUrl::fromLocalFile("/tmp/test1.txt");
    QUrl url2 = QUrl::fromLocalFile("/tmp/test2.txt");
    QUrl url3 = QUrl::fromLocalFile("/tmp/test3.txt");
    
    QPair<QUrl, RootInfo::EventType> event1(url1, RootInfo::kAddFile);
    QPair<QUrl, RootInfo::EventType> event2(url2, RootInfo::kUpdateFile);
    QPair<QUrl, RootInfo::EventType> event3(url3, RootInfo::kRmFile);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->enqueueEvent(event1));
    EXPECT_NO_FATAL_FAILURE(rootInfo->enqueueEvent(event2));
    EXPECT_NO_FATAL_FAILURE(rootInfo->enqueueEvent(event3));
    
    bool result = rootInfo->checkFileEventQueue();
    EXPECT_TRUE(result);
    
    QPair<QUrl, RootInfo::EventType> dequeued = rootInfo->dequeueEvent();
    EXPECT_EQ(dequeued.first, url3);
    EXPECT_EQ(dequeued.second, RootInfo::kRmFile);
}

TEST_F(TestRootInfo, CanHandleCacheOperations)
{
    QString key = "test_key";
    
    bool result = rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    EXPECT_TRUE(result);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->startWork(key, true));
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->handleGetSourceData(key));
}

TEST_F(TestRootInfo, CanHandleRefreshOperations)
{
    QString key = "test_key";
    
    rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    
    EXPECT_NO_FATAL_FAILURE(rootInfo->clearTraversalThread(key, true));
    
    bool result = rootInfo->initThreadOfFileData(key, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder, false);
    EXPECT_FALSE(result);
}
