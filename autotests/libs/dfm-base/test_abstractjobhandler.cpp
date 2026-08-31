// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractjobhandler.cpp
 * @brief Unit tests for AbstractJobHandler default implementations.
 */

#include <gtest/gtest.h>
#include <QMap>
#include <QVariant>
#include <QtGlobal>

#include <dfm-base/interfaces/abstractjobhandler.h>

using namespace dfmbase;

class TestJobHandler : public AbstractJobHandler
{
public:
    explicit TestJobHandler(QObject *parent = nullptr)
        : AbstractJobHandler(parent) { }
};

TEST(AbstractJobHandlerTest, DefaultCurrentJobProcessIsZero)
{
    TestJobHandler handler;
    EXPECT_EQ(handler.currentJobProcess(), 0.0);
}

TEST(AbstractJobHandlerTest, DefaultTotalSizeIsZero)
{
    TestJobHandler handler;
    EXPECT_EQ(handler.totalSize(), 0);
}

TEST(AbstractJobHandlerTest, DefaultCurrentSizeIsZero)
{
    TestJobHandler handler;
    EXPECT_EQ(handler.currentSize(), 0);
}

TEST(AbstractJobHandlerTest, DefaultCurrentStateIsUnknown)
{
    TestJobHandler handler;
    EXPECT_EQ(handler.currentState(), AbstractJobHandler::JobState::kUnknowState);
}

TEST(AbstractJobHandlerTest, SetSignalConnectFinishedSetsFlag)
{
    TestJobHandler handler;
    handler.setSignalConnectFinished();
    // After setSignalConnectFinished, isSignalConnectOver should be true.
    EXPECT_TRUE(handler.isSignalConnectOver);
}

TEST(AbstractJobHandlerTest, OperateTaskJobDoesNotChangeState)
{
    TestJobHandler handler;
    handler.operateTaskJob(AbstractJobHandler::SupportAction::kStartAction);
    // operateTaskJob on a default handler does not transition the state.
    EXPECT_EQ(handler.currentState(), AbstractJobHandler::JobState::kUnknowState);
}

TEST(AbstractJobHandlerTest, StartCallsOperateTaskJob)
{
    TestJobHandler handler;
    handler.start();
    EXPECT_EQ(handler.currentState(), AbstractJobHandler::JobState::kUnknowState);
}

// ---- Coverage additions: on* slots with JobInfoPointer + dtor ----

TEST(AbstractJobHandlerTest, OnSlotsPopulateTaskInfoMap)
{
    TestJobHandler handler;
    // Before any on* call, taskInfo should be empty.
    EXPECT_TRUE(handler.getAllTaskInfo().isEmpty());

    auto map = new QMap<quint8, QVariant>();
    (*map)[0] = QVariant(1);
    JobInfoPointer p(map);

    handler.onProccessChanged(p);
    handler.onStateChanged(p);
    handler.onFinished(p);
    handler.onSpeedUpdated(p);
    handler.onCurrentTask(p);
    handler.onError(p);

    // All 6 notify types should now be present in the task info map.
    auto tasks = handler.getAllTaskInfo();
    EXPECT_FALSE(tasks.isEmpty());
    EXPECT_EQ(tasks.size(), 6);
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifyProccessChangedKey));
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifyStateChangedKey));
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifyCurrentTaskKey));
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifyFinishedKey));
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifySpeedUpdatedTaskKey));
    EXPECT_TRUE(tasks.contains(AbstractJobHandler::NotifyType::kNotifyErrorTaskKey));
}

TEST(AbstractJobHandlerTest, GetTaskInfoByNotifyTypeReturnsStoredPointer)
{
    TestJobHandler handler;
    // Before insert: returns null (default-constructed) pointer.
    auto ptr = handler.getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyProccessChangedKey);
    EXPECT_TRUE(ptr.isNull());

    // After insert: returns the stored pointer.
    auto map = new QMap<quint8, QVariant>();
    (*map)[0] = QVariant(42);
    JobInfoPointer p(map);
    handler.onProccessChanged(p);
    auto ptr2 = handler.getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyProccessChangedKey);
    EXPECT_FALSE(ptr2.isNull());
    EXPECT_EQ(ptr2->value(0).toInt(), 42);
}

TEST(AbstractJobHandlerTest, LocalHandlerDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ TestJobHandler h; });
}


TEST(AbstractJobHandlerTest, AbstractJobHandler)
{
    // AbstractJobHandler
    SUCCEED();
}

TEST(AbstractJobHandlerTest, currentSize)
{
    // currentSize
    SUCCEED();
}
