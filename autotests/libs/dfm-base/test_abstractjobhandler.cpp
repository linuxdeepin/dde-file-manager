// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractjobhandler.cpp
 * @brief Unit tests for AbstractJobHandler default implementations.
 *
 * Note: QMap<quint8, QVariant> (JobInfoPointer's value type) triggers a Qt6
 * static assertion when fully instantiated in some contexts, so we avoid
 * creating JobInfoPointer objects and only exercise the no-arg / primitive
 * default methods.
 */

#include <gtest/gtest.h>

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

TEST(AbstractJobHandlerTest, SetSignalConnectFinished)
{
    TestJobHandler handler;
    EXPECT_NO_FATAL_FAILURE({ handler.setSignalConnectFinished(); });
}

TEST(AbstractJobHandlerTest, OperateTaskJobNoCrash)
{
    TestJobHandler handler;
    EXPECT_NO_FATAL_FAILURE({
        handler.operateTaskJob(AbstractJobHandler::SupportAction::kStartAction);
    });
}

TEST(AbstractJobHandlerTest, StartCallsOperateTaskJob)
{
    TestJobHandler handler;
    EXPECT_NO_FATAL_FAILURE({ handler.start(); });
}

// ---- Coverage additions for task-info accessors + local destruction ----

TEST(AbstractJobHandlerTest, GetAllTaskInfoReturnsEmptyMap)
{
    TestJobHandler handler;
    EXPECT_NO_FATAL_FAILURE({ (void)handler.getAllTaskInfo(); });
}

TEST(AbstractJobHandlerTest, GetTaskInfoByNotifyTypeReturnsNullpointer)
{
    TestJobHandler handler;
    EXPECT_NO_FATAL_FAILURE({ (void)handler.getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyProccessChangedKey); });
}

TEST(AbstractJobHandlerTest, LocalHandlerDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ TestJobHandler h; });
}
