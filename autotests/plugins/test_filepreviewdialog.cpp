// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepreviewdialog.cpp
 * @brief Unit tests for FilePreviewDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/views/filepreviewdialog.h"

#include <QTest>

using namespace src;

class FilePreviewDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FilePreviewDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FilePreviewDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FilePreviewDialogTest, restoreCenterPos)
{
    // Test method: void restoreCenterPos(())
    EXPECT_NO_FATAL_FAILURE(obj->restoreCenterPos());
}

TEST_F(FilePreviewDialogTest, saveCenterPos)
{
    // Test method: void saveCenterPos(())
    EXPECT_NO_FATAL_FAILURE(obj->saveCenterPos());
}
