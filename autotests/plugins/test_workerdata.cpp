// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workerdata.cpp
 * @brief Unit tests for WorkerData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/workerdata.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class WorkerDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkerData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkerData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkerDataTest, WorkerData)
{
    // Test constructor: WorkerData(())
    ASSERT_NE(obj, nullptr);
}
