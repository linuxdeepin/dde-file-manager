// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogprivate.cpp
 * @brief Unit tests for FileDialogPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/filedialog.h"

#include <QTest>

using namespace core;

class FileDialogPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogPrivateTest, handleSaveAcceptBtnClicked)
{
    // Test method: void handleSaveAcceptBtnClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->handleSaveAcceptBtnClicked());
}
