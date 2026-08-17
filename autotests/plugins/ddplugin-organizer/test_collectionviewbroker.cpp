// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "broker/collectionviewbroker.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "mode/collectiondataprovider.h"
#include "mode/custom/customdatahandler.h"

#include <QUrl>
#include <QRect>
#include <QPoint>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_CollectionViewBroker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&CollectionViewPrivate::initUI, []() {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&CollectionViewPrivate::initConnect, []() {
            __DBG_STUB_INVOKE__
        });

        provider = new CustomDataHandler();
        view = new CollectionView("test_uuid", provider);
        broker = new CollectionViewBroker(view);
    }

    virtual void TearDown() override
    {
        delete broker;
        broker = nullptr;
        delete view;
        view = nullptr;
        delete provider;
        provider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionViewBroker *broker = nullptr;
    CollectionView *view = nullptr;
    CollectionDataProvider *provider = nullptr;
};

TEST_F(UT_CollectionViewBroker, Constructor_WithParent_InitializesView)
{
    EXPECT_NE(broker, nullptr);
    EXPECT_EQ(broker->getView(), view);
    EXPECT_EQ(broker->parent(), view);
}

TEST_F(UT_CollectionViewBroker, Constructor_WithNullParent_ViewIsNull)
{
    CollectionViewBroker nullBroker(nullptr);
    EXPECT_EQ(nullBroker.getView(), nullptr);
}

TEST_F(UT_CollectionViewBroker, getView_ReturnsCorrectView)
{
    EXPECT_EQ(broker->getView(), view);
}

TEST_F(UT_CollectionViewBroker, setView_UpdatesViewAndParent)
{
    CollectionViewBroker testBroker(nullptr);
    EXPECT_EQ(testBroker.getView(), nullptr);

    testBroker.setView(view);
    EXPECT_EQ(testBroker.getView(), view);
    EXPECT_EQ(testBroker.parent(), view);
}

TEST_F(UT_CollectionViewBroker, setView_WithNull_SetsNull)
{
    broker->setView(nullptr);
    EXPECT_EQ(broker->getView(), nullptr);
}

TEST_F(UT_CollectionViewBroker, gridPoint_FileInList_ReturnsTrueAndSetsPos)
{
    QUrl testFile("file:///tmp/test.txt");
    QList<QUrl> mockItems;
    mockItems.append(testFile);
    QPoint expectedPos(2, 3);

    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    stub.set_lamda(&CollectionViewPrivate::nodeToPos, [&expectedPos](CollectionViewPrivate *, int node) {
        __DBG_STUB_INVOKE__
        return expectedPos;
    });

    QPoint resultPos;
    bool result = broker->gridPoint(testFile, resultPos);

    EXPECT_TRUE(result);
    EXPECT_EQ(resultPos, expectedPos);
}

TEST_F(UT_CollectionViewBroker, gridPoint_FileNotInList_ReturnsFalse)
{
    QUrl testFile("file:///tmp/nonexistent.txt");
    QList<QUrl> mockItems;
    mockItems.append(QUrl("file:///tmp/other.txt"));

    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    QPoint resultPos(999, 999);
    bool result = broker->gridPoint(testFile, resultPos);

    EXPECT_FALSE(result);
}

TEST_F(UT_CollectionViewBroker, gridPoint_EmptyList_ReturnsFalse)
{
    QUrl testFile("file:///tmp/test.txt");
    QList<QUrl> emptyItems;

    stub.set_lamda(VADDR(CustomDataHandler, items), [&emptyItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return emptyItems;
    });

    QPoint resultPos;
    bool result = broker->gridPoint(testFile, resultPos);

    EXPECT_FALSE(result);
}

TEST_F(UT_CollectionViewBroker, gridPoint_MultipleFiles_FindsCorrectIndex)
{
    QUrl file1("file:///tmp/file1.txt");
    QUrl file2("file:///tmp/file2.txt");
    QUrl file3("file:///tmp/file3.txt");
    QList<QUrl> mockItems;
    mockItems << file1 << file2 << file3;

    int calledWithNode = -1;
    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    stub.set_lamda(&CollectionViewPrivate::nodeToPos, [&calledWithNode](CollectionViewPrivate *, int node) {
        __DBG_STUB_INVOKE__
        calledWithNode = node;
        return QPoint(node, node);
    });

    QPoint resultPos;
    broker->gridPoint(file2, resultPos);

    EXPECT_EQ(calledWithNode, 1);
    EXPECT_EQ(resultPos, QPoint(1, 1));
}

TEST_F(UT_CollectionViewBroker, visualRect_FileInList_ReturnsRect)
{
    QUrl testFile("file:///tmp/test.txt");
    QList<QUrl> mockItems;
    mockItems.append(testFile);
    QPoint nodePos(1, 2);
    QRect expectedRect(10, 20, 100, 100);

    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    stub.set_lamda(&CollectionViewPrivate::nodeToPos, [&nodePos](CollectionViewPrivate *, int) {
        __DBG_STUB_INVOKE__
        return nodePos;
    });

    stub.set_lamda(&CollectionViewPrivate::visualRect, [&expectedRect](CollectionViewPrivate *, const QPoint &) {
        __DBG_STUB_INVOKE__
        return expectedRect;
    });

    QRect result = broker->visualRect(testFile);

    EXPECT_EQ(result, expectedRect);
}

TEST_F(UT_CollectionViewBroker, visualRect_FileNotInList_ReturnsEmptyRect)
{
    QUrl testFile("file:///tmp/nonexistent.txt");
    QList<QUrl> mockItems;
    mockItems.append(QUrl("file:///tmp/other.txt"));

    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    QRect result = broker->visualRect(testFile);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CollectionViewBroker, visualRect_EmptyList_ReturnsEmptyRect)
{
    QUrl testFile("file:///tmp/test.txt");
    QList<QUrl> emptyItems;

    stub.set_lamda(VADDR(CustomDataHandler, items), [&emptyItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return emptyItems;
    });

    QRect result = broker->visualRect(testFile);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CollectionViewBroker, visualRect_CallsNodeToPosWithCorrectNode)
{
    QUrl file1("file:///tmp/file1.txt");
    QUrl file2("file:///tmp/file2.txt");
    QList<QUrl> mockItems;
    mockItems << file1 << file2;

    int calledWithNode = -1;
    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    stub.set_lamda(&CollectionViewPrivate::nodeToPos, [&calledWithNode](CollectionViewPrivate *, int node) {
        __DBG_STUB_INVOKE__
        calledWithNode = node;
        return QPoint(0, 0);
    });

    stub.set_lamda(&CollectionViewPrivate::visualRect, [](CollectionViewPrivate *, const QPoint &) {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 50, 50);
    });

    broker->visualRect(file2);

    EXPECT_EQ(calledWithNode, 1);
}

TEST_F(UT_CollectionViewBroker, visualRect_PassesPosToVisualRect)
{
    QUrl testFile("file:///tmp/test.txt");
    QList<QUrl> mockItems;
    mockItems.append(testFile);
    QPoint expectedPos(5, 10);
    QPoint receivedPos;

    stub.set_lamda(VADDR(CustomDataHandler, items), [&mockItems](CollectionDataProvider *, const QString &) {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    stub.set_lamda(&CollectionViewPrivate::nodeToPos, [&expectedPos](CollectionViewPrivate *, int) {
        __DBG_STUB_INVOKE__
        return expectedPos;
    });

    stub.set_lamda(&CollectionViewPrivate::visualRect, [&receivedPos](CollectionViewPrivate *, const QPoint &pos) {
        __DBG_STUB_INVOKE__
        receivedPos = pos;
        return QRect(0, 0, 50, 50);
    });

    broker->visualRect(testFile);

    EXPECT_EQ(receivedPos, expectedPos);
}
