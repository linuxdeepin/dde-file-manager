// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanager1.cpp
 * @brief Unit tests for FileManager1 methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "filemanager1.h"

#include <QTest>

using namespace filemanager1;

class FileManager1Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManager1();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManager1 *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManager1Test, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
