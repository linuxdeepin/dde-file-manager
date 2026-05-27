// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "broker/organizerbroker.h"

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QRect>
#include <QPoint>
#include <QAbstractItemView>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class MockOrganizerBroker : public OrganizerBroker
{
public:
    explicit MockOrganizerBroker(QObject *parent = nullptr)
        : OrganizerBroker(parent) {}

    void refreshModel(bool global, int ms, bool file) override
    {
        refreshModelCalled = true;
        lastGlobal = global;
        lastMs = ms;
        lastFile = file;
    }

    QString gridPoint(const QUrl &item, QPoint *point) override
    {
        gridPointCalled = true;
        lastGridPointUrl = item;
        if (point)
            *point = mockPoint;
        return mockGridPointResult;
    }

    QRect visualRect(const QString &id, const QUrl &item) override
    {
        visualRectCalled = true;
        lastVisualRectId = id;
        lastVisualRectUrl = item;
        return mockVisualRect;
    }

    QAbstractItemView *view(const QString &id) override
    {
        viewCalled = true;
        lastViewId = id;
        return mockView;
    }

    QRect iconRect(const QString &id, QRect vrect) override
    {
        iconRectCalled = true;
        lastIconRectId = id;
        lastIconRectVRect = vrect;
        return mockIconRect;
    }

    bool selectAllItems() override
    {
        selectAllItemsCalled = true;
        return mockSelectAllResult;
    }

    // Test state tracking
    bool refreshModelCalled = false;
    bool lastGlobal = false;
    int lastMs = 0;
    bool lastFile = false;

    bool gridPointCalled = false;
    QUrl lastGridPointUrl;
    QPoint mockPoint;
    QString mockGridPointResult;

    bool visualRectCalled = false;
    QString lastVisualRectId;
    QUrl lastVisualRectUrl;
    QRect mockVisualRect;

    bool viewCalled = false;
    QString lastViewId;
    QAbstractItemView *mockView = nullptr;

    bool iconRectCalled = false;
    QString lastIconRectId;
    QRect lastIconRectVRect;
    QRect mockIconRect;

    bool selectAllItemsCalled = false;
    bool mockSelectAllResult = true;
};

class UT_OrganizerBroker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        broker = new MockOrganizerBroker();
    }

    virtual void TearDown() override
    {
        delete broker;
        broker = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    MockOrganizerBroker *broker = nullptr;
};

TEST_F(UT_OrganizerBroker, Constructor_WithParent_InitializesCorrectly)
{
    QObject parent;
    MockOrganizerBroker brokerWithParent(&parent);
    EXPECT_EQ(brokerWithParent.parent(), &parent);
}

TEST_F(UT_OrganizerBroker, Constructor_WithNullParent_InitializesCorrectly)
{
    EXPECT_NE(broker, nullptr);
    EXPECT_EQ(broker->parent(), nullptr);
}

TEST_F(UT_OrganizerBroker, init_CallsSlotConnect_ReturnsTrue)
{
    bool result = broker->init();
    EXPECT_TRUE(result);
}

TEST_F(UT_OrganizerBroker, refreshModel_WithGlobalTrue_CallsRefreshModel)
{
    broker->refreshModel(true, 100, false);

    EXPECT_TRUE(broker->refreshModelCalled);
    EXPECT_TRUE(broker->lastGlobal);
    EXPECT_EQ(broker->lastMs, 100);
    EXPECT_FALSE(broker->lastFile);
}

TEST_F(UT_OrganizerBroker, refreshModel_WithFileFlagTrue_CallsRefreshModel)
{
    broker->refreshModel(false, 200, true);

    EXPECT_TRUE(broker->refreshModelCalled);
    EXPECT_FALSE(broker->lastGlobal);
    EXPECT_EQ(broker->lastMs, 200);
    EXPECT_TRUE(broker->lastFile);
}

TEST_F(UT_OrganizerBroker, gridPoint_WithValidUrl_ReturnsId)
{
    QUrl testUrl("file:///tmp/test.txt");
    QPoint expectedPoint(5, 10);
    QString expectedResult = "collection_1";

    broker->mockPoint = expectedPoint;
    broker->mockGridPointResult = expectedResult;

    QPoint resultPoint;
    QString result = broker->gridPoint(testUrl, &resultPoint);

    EXPECT_TRUE(broker->gridPointCalled);
    EXPECT_EQ(broker->lastGridPointUrl, testUrl);
    EXPECT_EQ(result, expectedResult);
    EXPECT_EQ(resultPoint, expectedPoint);
}

TEST_F(UT_OrganizerBroker, gridPoint_WithNullPoint_DoesNotCrash)
{
    QUrl testUrl("file:///tmp/test.txt");
    broker->mockGridPointResult = "collection_2";

    QString result = broker->gridPoint(testUrl, nullptr);

    EXPECT_TRUE(broker->gridPointCalled);
    EXPECT_EQ(result, "collection_2");
}

TEST_F(UT_OrganizerBroker, visualRect_WithValidParams_ReturnsRect)
{
    QString testId = "view_1";
    QUrl testUrl("file:///tmp/test.txt");
    QRect expectedRect(100, 200, 50, 50);

    broker->mockVisualRect = expectedRect;

    QRect result = broker->visualRect(testId, testUrl);

    EXPECT_TRUE(broker->visualRectCalled);
    EXPECT_EQ(broker->lastVisualRectId, testId);
    EXPECT_EQ(broker->lastVisualRectUrl, testUrl);
    EXPECT_EQ(result, expectedRect);
}

TEST_F(UT_OrganizerBroker, view_WithValidId_ReturnsView)
{
    QString testId = "view_1";

    QAbstractItemView *result = broker->view(testId);

    EXPECT_TRUE(broker->viewCalled);
    EXPECT_EQ(broker->lastViewId, testId);
    EXPECT_EQ(result, broker->mockView);
}

TEST_F(UT_OrganizerBroker, iconRect_WithValidParams_ReturnsRect)
{
    QString testId = "view_1";
    QRect inputRect(10, 20, 100, 100);
    QRect expectedRect(15, 25, 48, 48);

    broker->mockIconRect = expectedRect;

    QRect result = broker->iconRect(testId, inputRect);

    EXPECT_TRUE(broker->iconRectCalled);
    EXPECT_EQ(broker->lastIconRectId, testId);
    EXPECT_EQ(broker->lastIconRectVRect, inputRect);
    EXPECT_EQ(result, expectedRect);
}

TEST_F(UT_OrganizerBroker, selectAllItems_ReturnsTrueWhenSuccessful)
{
    broker->mockSelectAllResult = true;

    bool result = broker->selectAllItems();

    EXPECT_TRUE(broker->selectAllItemsCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_OrganizerBroker, selectAllItems_ReturnsFalseWhenFailed)
{
    broker->mockSelectAllResult = false;

    bool result = broker->selectAllItems();

    EXPECT_TRUE(broker->selectAllItemsCalled);
    EXPECT_FALSE(result);
}
