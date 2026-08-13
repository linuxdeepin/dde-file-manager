// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_progressnotifier.cpp
 * @brief Unit tests for ProgressNotifier (task/progressnotifier.h)
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/progressnotifier.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class ProgressNotifierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp)
            app = std::make_unique<QCoreApplication>(argc, argv);
    }

    std::unique_ptr<QCoreApplication> app;
    static int argc;
    static char *argv[];
};

int ProgressNotifierTest::argc = 1;
char *ProgressNotifierTest::argv[] = { const_cast<char *>("test_progressnotifier") };

TEST_F(ProgressNotifierTest, InstanceReturnsNonNull)
{
    auto *inst = ProgressNotifier::instance();
    ASSERT_NE(inst, nullptr);
}

TEST_F(ProgressNotifierTest, InstanceReturnsSame)
{
    auto *a = ProgressNotifier::instance();
    auto *b = ProgressNotifier::instance();
    EXPECT_EQ(a, b);
}

TEST_F(ProgressNotifierTest, ConnectSignalAndEmit)
{
    auto *inst = ProgressNotifier::instance();
    QSignalSpy spy(inst, &ProgressNotifier::progressChanged);
    ASSERT_TRUE(spy.isValid());

    emit inst->progressChanged(42, 100);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 42);
    EXPECT_EQ(spy.at(0).at(1).toInt(), 100);
}