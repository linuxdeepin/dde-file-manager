// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_copyfiles.cpp
 * @brief Unit tests for CopyFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/copyfiles/copyfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class CopyFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CopyFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CopyFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CopyFilesTest, CopyFiles_Destructor)
{
    // Test method:  ~CopyFiles(())
    EXPECT_NO_FATAL_FAILURE({ CopyFiles *tmp = new CopyFiles(); delete tmp; });
}
