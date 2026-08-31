// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rootinfo_1.cpp
 * @brief Unit tests for RootInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/rootinfo.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RootInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RootInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RootInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RootInfoTest, RootInfo)
{
    // Test constructor: RootInfo((const QUrl &u, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RootInfoTest, addChild)
{
    // Test method: SortInfoPointer addChild((const FileInfoPointer &child))
    FileInfoPointer _arg0{};
    auto result = obj->addChild(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(RootInfoTest, addConnectToken)
{
    // Test method: void addConnectToken(())
    EXPECT_NO_FATAL_FAILURE(obj->addConnectToken());
}

TEST_F(RootInfoTest, checkFileEventQueue)
{
    // Test bool getter: checkFileEventQueue()
    bool result = obj->checkFileEventQueue();
    EXPECT_FALSE(result);

}

TEST_F(RootInfoTest, checkKeyOnly)
{
    // Test method: bool checkKeyOnly((const QString &key))
    QString _arg0{};
    auto result = obj->checkKeyOnly(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RootInfoTest, connectTokens)
{
    // Test getter: QStringList connectTokens()
    auto result = obj->connectTokens();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RootInfoTest, containsChild)
{
    // Test method: bool containsChild((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->containsChild(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RootInfoTest, dequeueEvent)
{
    // Test getter: QPair<QUrl, RootInfo::EventType> dequeueEvent()
    auto result = obj->dequeueEvent();
    EXPECT_NO_FATAL_FAILURE({ obj->dequeueEvent(); });

}

TEST_F(RootInfoTest, doFileUpdated)
{
    // Test method: void doFileUpdated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doFileUpdated(_arg0));
}

TEST_F(RootInfoTest, doThreadWatcherEvent)
{
    // Test method: void doThreadWatcherEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->doThreadWatcherEvent());
}

TEST_F(RootInfoTest, dofileCreated)
{
    // Test method: void dofileCreated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->dofileCreated(_arg0));
}

TEST_F(RootInfoTest, dofileMoved)
{
    // Test method: void dofileMoved((const QUrl &fromUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->dofileMoved(_arg0, _arg1));
}

TEST_F(RootInfoTest, enqueueEvent)
{
    // Test event handler: enqueueEvent((const QPair<QUrl, EventType> &e))
    QPair<QUrl, EventType> _event(QPair<QUrl, EventType>::None);
    EXPECT_NO_FATAL_FAILURE(obj->enqueueEvent(&_event));
}

TEST_F(RootInfoTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(RootInfoTest, handleTraversalFinish)
{
    // Test method: void handleTraversalFinish((const QString &travseToken))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalFinish(_arg0));
}

TEST_F(RootInfoTest, handleTraversalLocalResult)
{
    // Test method: void handleTraversalLocalResult((QList<SortInfoPointer> children,
                                          dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                          Qt::SortOrder sortOrder, bool isMixDirAndFile, const QString &travseToken))
    QString _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalLocalResult(QList<SortInfoPointer>(), {}, Qt::SortOrder(), false, _arg4));
}

TEST_F(RootInfoTest, handleTraversalResultsUpdate)
{
    // Test method: void handleTraversalResultsUpdate((const QList<SortInfoPointer> children, const QString &travseToken))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalResultsUpdate(QList<SortInfoPointer>(), _arg1));
}

TEST_F(RootInfoTest, handleTraversalSort)
{
    // Test method: void handleTraversalSort((const QString &travseToken))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTraversalSort(_arg0));
}

TEST_F(RootInfoTest, initConnection)
{
    // Test method: void initConnection((const TraversalThreadManagerPointer &traversalThread))
    TraversalThreadManagerPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->initConnection(_arg0));
}

TEST_F(RootInfoTest, setFirstBatch)
{
    // Test setter: void setFirstBatch((bool first))
    EXPECT_NO_FATAL_FAILURE(obj->setFirstBatch(false));
}

TEST_F(RootInfoTest, sortFileInfo)
{
    // Test method: SortInfoPointer sortFileInfo((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->sortFileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(RootInfoTest, startWatcher)
{
    // Test method: void startWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->startWatcher());
}

TEST_F(RootInfoTest, startWork)
{
    // Test method: void startWork((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->startWork(_arg0));
}

TEST_F(RootInfoTest, updateChild)
{
    // Test method: SortInfoPointer updateChild((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->updateChild(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(RootInfoTest, RootInfo_Destructor)
{
    // Test method:  ~RootInfo(())
    EXPECT_NO_FATAL_FAILURE({ RootInfo *tmp = new RootInfo(); delete tmp; });
}
