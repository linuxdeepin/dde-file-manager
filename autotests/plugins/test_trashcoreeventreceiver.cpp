// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcoreeventreceiver.cpp
 * @brief Unit tests for TrashCoreEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/trashcoreeventreceiver.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreEventReceiverTest, copyFromFile)
{
    // Test method: bool copyFromFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags))
    auto result = obj->copyFromFile(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(TrashCoreEventReceiverTest, cutFileFromTrash)
{
    // Test method: bool cutFileFromTrash((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags))
    auto result = obj->cutFileFromTrash(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(TrashCoreEventReceiverTest, handleEmptyTrash)
{
    // Test method: void handleEmptyTrash((const quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleEmptyTrash(0));
}

TEST_F(TrashCoreEventReceiverTest, instance)
{
    // Test getter: TrashCoreEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
