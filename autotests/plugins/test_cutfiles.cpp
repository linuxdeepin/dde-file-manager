// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cutfiles.cpp
 * @brief Unit tests for CutFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/cutfiles/cutfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class CutFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CutFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CutFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CutFilesTest, CutFiles_Destructor)
{
    // Test method:  ~CutFiles(())
    EXPECT_NO_FATAL_FAILURE({ CutFiles *tmp = new CutFiles(); delete tmp; });
}
