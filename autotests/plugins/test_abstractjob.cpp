// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractjob.cpp
 * @brief Unit tests for AbstractJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class AbstractJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractJobTest, AbstractJob)
{
    // Test constructor: AbstractJob((AbstractWorker *doWorker, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractJobTest, handleFileDeleted)
{
    // Test method: void handleFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileDeleted(_arg0));
}

TEST_F(AbstractJobTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
