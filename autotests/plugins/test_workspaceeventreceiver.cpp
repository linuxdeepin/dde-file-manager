// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventreceiver.cpp
 * @brief Unit tests for WorkspaceEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/workspaceeventreceiver.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventReceiverTest, handleClosePersistentEditor)
{
    // Test method: void handleClosePersistentEditor((const quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleClosePersistentEditor(0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisteredGroupStrategies)
{
    // Test method: QVariantList handleRegisteredGroupStrategies((const QString &schemeFilter))
    QString _arg0{};
    auto result = obj->handleRegisteredGroupStrategies(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleTabRemoved)
{
    // Test method: void handleTabRemoved((const quint64 windowId, const QString &removedId, const QString &nextId))
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTabRemoved(0, _arg1, _arg2));
}

TEST_F(WorkspaceEventReceiverTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}
