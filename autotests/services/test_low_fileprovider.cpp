// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fileprovider.cpp
 * @brief Unit tests for FileProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class FileProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileProviderTest, totalCount)
{
    // Test getter: qint64 totalCount()
    auto result = obj->totalCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileProviderTest, FileProvider_Destructor)
{
    // Test method:  ~FileProvider(())
    EXPECT_NO_FATAL_FAILURE({ FileProvider *tmp = new FileProvider(); delete tmp; });
}
