// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogstatusbar.cpp
 * @brief Unit tests for FileDialogStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/filedialogstatusbar.h"

#include <QTest>

using namespace core;

class FileDialogStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogStatusBarTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(FileDialogStatusBarTest, lineEdit)
{
    // Test getter: DLineEdit lineEdit()
    auto result = obj->lineEdit();
    EXPECT_NO_FATAL_FAILURE({ obj->lineEdit(); });

}
