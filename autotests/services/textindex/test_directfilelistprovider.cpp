// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_directfilelistprovider.cpp
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

TEST_F(DirectFileListProviderTest, totalCount)
{
    // Test getter: qint64 totalCount()
    auto result = obj->totalCount();
    EXPECT_EQ(result, 0);

}
