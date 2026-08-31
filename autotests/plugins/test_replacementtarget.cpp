// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replacementtarget.cpp
 * @brief Unit tests for ReplacementTarget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class ReplacementTargetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ReplacementTarget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ReplacementTarget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ReplacementTargetTest, isUsingTemporary)
{
    // Test bool getter: isUsingTemporary()
    bool result = obj->isUsingTemporary();
    EXPECT_FALSE(result);

}
