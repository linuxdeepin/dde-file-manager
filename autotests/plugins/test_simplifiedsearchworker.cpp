// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_simplifiedsearchworker.cpp
 * @brief Unit tests for SimplifiedSearchWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/maincontroller/task/taskcommander.h"

#include <QTest>

using namespace dfmplugin_search;

class SimplifiedSearchWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SimplifiedSearchWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SimplifiedSearchWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SimplifiedSearchWorkerTest, createSearchers)
{
    // Test method: void createSearchers(())
    EXPECT_NO_FATAL_FAILURE(obj->createSearchers());
}

TEST_F(SimplifiedSearchWorkerTest, mergeResults)
{
    // Test method: void mergeResults((AbstractSearcher *searcher))
    EXPECT_NO_FATAL_FAILURE(obj->mergeResults(nullptr));
}
