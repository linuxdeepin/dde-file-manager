// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indexprofile.cpp
 * @brief Unit tests for IndexProfile methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/profile/indexprofile.h"

#include <QTest>

using namespace src;

class IndexProfileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexProfile();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexProfile *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexProfileTest, supportsChecksum)
{
    // Test bool getter: supportsChecksum()
    bool result = obj->supportsChecksum();
    EXPECT_FALSE(result);

}
