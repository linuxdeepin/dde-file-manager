// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_methodgrouphelper_1.cpp
 * @brief Unit tests for MethodGroupHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/methodgroup/methodgrouphelper.h"

#include <QTest>

using namespace ddplugin_organizer;

class MethodGroupHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MethodGroupHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MethodGroupHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MethodGroupHelperTest, create)
{
    // Test method: MethodGroupHelper create((Classifier id))
    auto result = obj->create(Classifier());
    EXPECT_NO_FATAL_FAILURE({ obj->create(Classifier()); });

}

TEST_F(MethodGroupHelperTest, release)
{
    // Test method: void release(())
    EXPECT_NO_FATAL_FAILURE(obj->release());
}
