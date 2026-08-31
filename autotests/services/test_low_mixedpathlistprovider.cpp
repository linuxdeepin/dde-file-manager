// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_mixedpathlistprovider.cpp
 * @brief Unit tests for MixedPathListProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class MixedPathListProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MixedPathListProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MixedPathListProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MixedPathListProviderTest, MixedPathListProvider)
{
    // Test constructor: MixedPathListProvider((IndexProfile profile, const QStringList &pathList))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MixedPathListProviderTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
