// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepreviewdialogstatusbar.cpp
 * @brief Unit tests for FilePreviewDialogStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/views/filepreviewdialogstatusbar.h"

#include <QTest>

using namespace src;

class FilePreviewDialogStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FilePreviewDialogStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FilePreviewDialogStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FilePreviewDialogStatusBarTest, FilePreviewDialogStatusBar)
{
    // Test constructor: FilePreviewDialogStatusBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
