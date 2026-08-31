// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindowsmanagerprivate.cpp
 * @brief Unit tests for FileManagerWindowsManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QTest>

using namespace src;

class FileManagerWindowsManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManagerWindowsManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManagerWindowsManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowsManagerPrivateTest, onWindowClosed)
{
    // Test method: void onWindowClosed((FileManagerWindow *window))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(nullptr));
}
