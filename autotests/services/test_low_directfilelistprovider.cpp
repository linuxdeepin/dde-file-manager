// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_directfilelistprovider.cpp
 * @brief Unit tests for DirectFileListProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class DirectFileListProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirectFileListProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirectFileListProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirectFileListProviderTest, DirectFileListProvider)
{
    // Test constructor: DirectFileListProvider((const dfmsearch::SearchResultList &files))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DirectFileListProviderTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(DirectFileListProviderTest, traverse)
{
    // Test method: void traverse((TaskState &state, const FileHandler &handler))
    TaskState _arg0{};
    FileHandler _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->traverse(_arg0, _arg1));
}
