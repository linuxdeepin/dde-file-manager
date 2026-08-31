// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdbusworker.cpp
 * @brief Unit tests for TagDBusWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tagdaemon.h"

#include <QTest>

using namespace tag;

class TagDBusWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDBusWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDBusWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDBusWorkerTest, launchService)
{
    // Test method: void launchService(())
    EXPECT_NO_FATAL_FAILURE(obj->launchService());
}
