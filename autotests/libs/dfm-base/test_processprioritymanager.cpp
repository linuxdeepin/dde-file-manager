// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processprioritymanager.cpp
 * @brief Unit tests for ProcessPriorityManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/processprioritymanager.h"

#include <QTest>

using namespace src;

class ProcessPriorityManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProcessPriorityManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProcessPriorityManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProcessPriorityManagerTest, lowerCpuNicePriority)
{
    // Test method: bool lowerCpuNicePriority((int niceValue))
    auto result = obj->lowerCpuNicePriority(0);
    EXPECT_FALSE(result);

}

TEST_F(ProcessPriorityManagerTest, lowerIoPriority)
{
    // Test bool getter: lowerIoPriority()
    bool result = obj->lowerIoPriority();
    EXPECT_FALSE(result);

}
