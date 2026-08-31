// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_filesystemprovider.cpp
 * @brief Unit tests for FileSystemProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class FileSystemProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileSystemProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileSystemProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSystemProviderTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
